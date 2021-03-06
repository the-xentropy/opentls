/*
 * Copyright 2006-2016 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <opentls/objects.h>
#include <opentls/evp.h>
#include "internal/cryptlib.h"
#include "crypto/evp.h"
#include "internal/provider.h"
#include "evp_local.h"

static int evp_pkey_asym_cipher_init(EVP_PKEY_CTX *ctx, int operation)
{
    int ret = 0;
    void *provkey = NULL;
    EVP_ASYM_CIPHER *cipher = NULL;

    if (ctx == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    evp_pkey_ctx_free_old_ops(ctx);
    ctx->operation = operation;

    if (ctx->keytype == NULL || ctx->engine != NULL)
        goto legacy;

    if (ctx->keymgmt == NULL)
        ctx->keymgmt =
            EVP_KEYMGMT_fetch(ctx->libctx, ctx->keytype, ctx->propquery);
    if (ctx->keymgmt != NULL) {
        const char *supported_ciph = NULL;

        if (ctx->keymgmt->query_operation_name != NULL)
            supported_ciph =
                ctx->keymgmt->query_operation_name(Otls_OP_ASYM_CIPHER);

        /*
         * If we didn't get a supported ciph, assume there is one with the
         * same name as the key type.
         */
        if (supported_ciph == NULL)
            supported_ciph = ctx->keytype;

        /*
         * Because we cleared out old ops, we shouldn't need to worry about
         * checking if cipher is already there.
         */
        cipher =
            EVP_ASYM_CIPHER_fetch(ctx->libctx, supported_ciph, ctx->propquery);
    }

    if (ctx->keymgmt == NULL
        || cipher == NULL
        || (EVP_KEYMGMT_provider(ctx->keymgmt)
            != EVP_ASYM_CIPHER_provider(cipher))) {
        /*
         * We don't have the full support we need with provided methods,
         * let's go see if legacy does.  Also, we don't need to free
         * ctx->keymgmt here, as it's not necessarily tied to this
         * operation.  It will be freed by EVP_PKEY_CTX_free().
         */
        EVP_ASYM_CIPHER_free(cipher);
        goto legacy;
    }

    ctx->op.ciph.cipher = cipher;

    if (ctx->pkey != NULL) {
        provkey = evp_keymgmt_export_to_provider(ctx->pkey, ctx->keymgmt, 0);
        /* If export failed, legacy may be able to pick it up */
        if (provkey == NULL)
            goto legacy;
    }
    ctx->op.ciph.ciphprovctx = cipher->newctx(otls_provider_ctx(cipher->prov));
    if (ctx->op.ciph.ciphprovctx == NULL) {
        /* The provider key can stay in the cache */
        EVPerr(0, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    switch (operation) {
    case EVP_PKEY_OP_ENCRYPT:
        if (cipher->encrypt_init == NULL) {
            EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            ret = -2;
            goto err;
        }
        ret = cipher->encrypt_init(ctx->op.ciph.ciphprovctx, provkey);
        break;
    case EVP_PKEY_OP_DECRYPT:
        if (cipher->decrypt_init == NULL) {
            EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            ret = -2;
            goto err;
        }
        ret = cipher->decrypt_init(ctx->op.ciph.ciphprovctx, provkey);
        break;
    default:
        EVPerr(0, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    if (ret <= 0) {
        cipher->freectx(ctx->op.ciph.ciphprovctx);
        ctx->op.ciph.ciphprovctx = NULL;
        goto err;
    }
    return 1;

 legacy:
    if (ctx->pmeth == NULL || ctx->pmeth->encrypt == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    switch(ctx->operation) {
    case EVP_PKEY_OP_ENCRYPT:
        if (ctx->pmeth->encrypt_init == NULL)
            return 1;
        ret = ctx->pmeth->encrypt_init(ctx);
        break;
    case EVP_PKEY_OP_DECRYPT:
        if (ctx->pmeth->decrypt_init == NULL)
            return 1;
        ret = ctx->pmeth->decrypt_init(ctx);
        break;
    default:
        EVPerr(0, EVP_R_INITIALIZATION_ERROR);
        ret = -1;
    }

 err:
    if (ret <= 0)
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    return ret;
}

int EVP_PKEY_encrypt_init(EVP_PKEY_CTX *ctx)
{
    return evp_pkey_asym_cipher_init(ctx, EVP_PKEY_OP_ENCRYPT);
}

int EVP_PKEY_encrypt(EVP_PKEY_CTX *ctx,
                     unsigned char *out, size_t *outlen,
                     const unsigned char *in, size_t inlen)
{
    int ret;

    if (ctx == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (ctx->operation != EVP_PKEY_OP_ENCRYPT) {
        EVPerr(0, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    if (ctx->op.ciph.ciphprovctx == NULL)
        goto legacy;

    ret = ctx->op.ciph.cipher->encrypt(ctx->op.ciph.ciphprovctx, out, outlen,
                                       (out == NULL ? 0 : *outlen), in, inlen);
    return ret;

 legacy:
    if (ctx->pmeth == NULL || ctx->pmeth->encrypt == NULL) {
        EVPerr(EVP_F_EVP_PKEY_ENCRYPT,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    M_check_autoarg(ctx, out, outlen, EVP_F_EVP_PKEY_ENCRYPT)
        return ctx->pmeth->encrypt(ctx, out, outlen, in, inlen);
}

int EVP_PKEY_decrypt_init(EVP_PKEY_CTX *ctx)
{
    return evp_pkey_asym_cipher_init(ctx, EVP_PKEY_OP_DECRYPT);
}

int EVP_PKEY_decrypt(EVP_PKEY_CTX *ctx,
                     unsigned char *out, size_t *outlen,
                     const unsigned char *in, size_t inlen)
{
    int ret;

    if (ctx == NULL) {
        EVPerr(0, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }

    if (ctx->operation != EVP_PKEY_OP_DECRYPT) {
        EVPerr(0, EVP_R_OPERATON_NOT_INITIALIZED);
        return -1;
    }

    if (ctx->op.ciph.ciphprovctx == NULL)
        goto legacy;

    ret = ctx->op.ciph.cipher->decrypt(ctx->op.ciph.ciphprovctx, out, outlen,
                                       (out == NULL ? 0 : *outlen), in, inlen);
    return ret;

 legacy:
    if (ctx->pmeth == NULL || ctx->pmeth->decrypt == NULL) {
        EVPerr(EVP_F_EVP_PKEY_DECRYPT,
               EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
        return -2;
    }
    M_check_autoarg(ctx, out, outlen, EVP_F_EVP_PKEY_DECRYPT)
        return ctx->pmeth->decrypt(ctx, out, outlen, in, inlen);
}


static EVP_ASYM_CIPHER *evp_asym_cipher_new(Otls_PROVIDER *prov)
{
    EVP_ASYM_CIPHER *cipher = OPENtls_zalloc(sizeof(EVP_ASYM_CIPHER));

    if (cipher == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    cipher->lock = CRYPTO_THREAD_lock_new();
    if (cipher->lock == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        OPENtls_free(cipher);
        return NULL;
    }
    cipher->prov = prov;
    otls_provider_up_ref(prov);
    cipher->refcnt = 1;

    return cipher;
}

static void *evp_asym_cipher_from_dispatch(int name_id,
                                           const Otls_DISPATCH *fns,
                                           Otls_PROVIDER *prov)
{
    EVP_ASYM_CIPHER *cipher = NULL;
    int ctxfncnt = 0, encfncnt = 0, decfncnt = 0;
    int gparamfncnt = 0, sparamfncnt = 0;

    if ((cipher = evp_asym_cipher_new(prov)) == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    cipher->name_id = name_id;

    for (; fns->function_id != 0; fns++) {
        switch (fns->function_id) {
        case Otls_FUNC_ASYM_CIPHER_NEWCTX:
            if (cipher->newctx != NULL)
                break;
            cipher->newctx = Otls_get_OP_asym_cipher_newctx(fns);
            ctxfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_ENCRYPT_INIT:
            if (cipher->encrypt_init != NULL)
                break;
            cipher->encrypt_init = Otls_get_OP_asym_cipher_encrypt_init(fns);
            encfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_ENCRYPT:
            if (cipher->encrypt != NULL)
                break;
            cipher->encrypt = Otls_get_OP_asym_cipher_encrypt(fns);
            encfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_DECRYPT_INIT:
            if (cipher->decrypt_init != NULL)
                break;
            cipher->decrypt_init = Otls_get_OP_asym_cipher_decrypt_init(fns);
            decfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_DECRYPT:
            if (cipher->decrypt != NULL)
                break;
            cipher->decrypt = Otls_get_OP_asym_cipher_decrypt(fns);
            decfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_FREECTX:
            if (cipher->freectx != NULL)
                break;
            cipher->freectx = Otls_get_OP_asym_cipher_freectx(fns);
            ctxfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_DUPCTX:
            if (cipher->dupctx != NULL)
                break;
            cipher->dupctx = Otls_get_OP_asym_cipher_dupctx(fns);
            break;
        case Otls_FUNC_ASYM_CIPHER_GET_CTX_PARAMS:
            if (cipher->get_ctx_params != NULL)
                break;
            cipher->get_ctx_params
                = Otls_get_OP_asym_cipher_get_ctx_params(fns);
            gparamfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_GETTABLE_CTX_PARAMS:
            if (cipher->gettable_ctx_params != NULL)
                break;
            cipher->gettable_ctx_params
                = Otls_get_OP_asym_cipher_gettable_ctx_params(fns);
            gparamfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_SET_CTX_PARAMS:
            if (cipher->set_ctx_params != NULL)
                break;
            cipher->set_ctx_params
                = Otls_get_OP_asym_cipher_set_ctx_params(fns);
            sparamfncnt++;
            break;
        case Otls_FUNC_ASYM_CIPHER_SETTABLE_CTX_PARAMS:
            if (cipher->settable_ctx_params != NULL)
                break;
            cipher->settable_ctx_params
                = Otls_get_OP_asym_cipher_settable_ctx_params(fns);
            sparamfncnt++;
            break;
        }
    }
    if (ctxfncnt != 2
        || (encfncnt != 0 && encfncnt != 2)
        || (decfncnt != 0 && decfncnt != 2)
        || (encfncnt != 2 && decfncnt != 2)
        || (gparamfncnt != 0 && gparamfncnt != 2)
        || (sparamfncnt != 0 && sparamfncnt != 2)) {
        /*
         * In order to be a consistent set of functions we must have at least
         * a set of context functions (newctx and freectx) as well as a pair of
         * "cipher" functions: (encrypt_init, encrypt) or
         * (decrypt_init decrypt). set_ctx_params and settable_ctx_params are
         * optional, but if one of them is present then the other one must also
         * be present. The same applies to get_ctx_params and
         * gettable_ctx_params. The dupctx function is optional.
         */
        ERR_raise(ERR_LIB_EVP, EVP_R_INVALID_PROVIDER_FUNCTIONS);
        goto err;
    }

    return cipher;
 err:
    EVP_ASYM_CIPHER_free(cipher);
    return NULL;
}

void EVP_ASYM_CIPHER_free(EVP_ASYM_CIPHER *cipher)
{
    if (cipher != NULL) {
        int i;

        CRYPTO_DOWN_REF(&cipher->refcnt, &i, cipher->lock);
        if (i > 0)
            return;
        otls_provider_free(cipher->prov);
        CRYPTO_THREAD_lock_free(cipher->lock);
        OPENtls_free(cipher);
    }
}

int EVP_ASYM_CIPHER_up_ref(EVP_ASYM_CIPHER *cipher)
{
    int ref = 0;

    CRYPTO_UP_REF(&cipher->refcnt, &ref, cipher->lock);
    return 1;
}

Otls_PROVIDER *EVP_ASYM_CIPHER_provider(const EVP_ASYM_CIPHER *cipher)
{
    return cipher->prov;
}

EVP_ASYM_CIPHER *EVP_ASYM_CIPHER_fetch(OPENtls_CTX *ctx, const char *algorithm,
                                       const char *properties)
{
    return evp_generic_fetch(ctx, Otls_OP_ASYM_CIPHER, algorithm, properties,
                             evp_asym_cipher_from_dispatch,
                             (int (*)(void *))EVP_ASYM_CIPHER_up_ref,
                             (void (*)(void *))EVP_ASYM_CIPHER_free);
}

int EVP_ASYM_CIPHER_is_a(const EVP_ASYM_CIPHER *cipher, const char *name)
{
    return evp_is_a(cipher->prov, cipher->name_id, name);
}

int EVP_ASYM_CIPHER_number(const EVP_ASYM_CIPHER *cipher)
{
    return cipher->name_id;
}

void EVP_ASYM_CIPHER_do_all_provided(OPENtls_CTX *libctx,
                                     void (*fn)(EVP_ASYM_CIPHER *cipher,
                                                void *arg),
                                     void *arg)
{
    evp_generic_do_all(libctx, Otls_OP_ASYM_CIPHER,
                       (void (*)(void *, void *))fn, arg,
                       evp_asym_cipher_from_dispatch,
                       (void (*)(void *))EVP_ASYM_CIPHER_free);
}


void EVP_ASYM_CIPHER_names_do_all(const EVP_ASYM_CIPHER *cipher,
                                  void (*fn)(const char *name, void *data),
                                  void *data)
{
    if (cipher->prov != NULL)
        evp_names_do_all(cipher->prov, cipher->name_id, fn, data);
}

