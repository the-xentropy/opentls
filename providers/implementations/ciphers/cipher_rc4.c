/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

/* Dispatch functions for RC4 ciphers */

#include "cipher_rc4.h"
#include "prov/implementations.h"

/* TODO (3.0) Figure out what flags are required */
#define RC4_FLAGS EVP_CIPH_FLAG_DEFAULT_ASN1

static Otls_OP_cipher_freectx_fn rc4_freectx;
static Otls_OP_cipher_dupctx_fn rc4_dupctx;

static void rc4_freectx(void *vctx)
{
    PROV_RC4_CTX *ctx = (PROV_RC4_CTX *)vctx;

    OPENtls_clear_free(ctx,  sizeof(*ctx));
}

static void *rc4_dupctx(void *ctx)
{
    PROV_RC4_CTX *in = (PROV_RC4_CTX *)ctx;
    PROV_RC4_CTX *ret = OPENtls_malloc(sizeof(*ret));

    if (ret == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    *ret = *in;

    return ret;
}

#define IMPLEMENT_cipher(alg, UCALG, flags, kbits, blkbits, ivbits, typ)       \
static Otls_OP_cipher_get_params_fn alg##_##kbits##_get_params;                \
static int alg##_##kbits##_get_params(Otls_PARAM params[])                     \
{                                                                              \
    return cipher_generic_get_params(params, 0, flags,                         \
                                     kbits, blkbits, ivbits);                  \
}                                                                              \
static Otls_OP_cipher_newctx_fn alg##_##kbits##_newctx;                        \
static void * alg##_##kbits##_newctx(void *provctx)                            \
{                                                                              \
     PROV_##UCALG##_CTX *ctx = OPENtls_zalloc(sizeof(*ctx));                   \
     if (ctx != NULL) {                                                        \
         cipher_generic_initkey(ctx, kbits, blkbits, ivbits, 0, flags,         \
                                PROV_CIPHER_HW_##alg(kbits), NULL);            \
     }                                                                         \
     return ctx;                                                               \
}                                                                              \
const Otls_DISPATCH alg##kbits##_functions[] = {                               \
    { Otls_FUNC_CIPHER_NEWCTX,                                                 \
      (void (*)(void)) alg##_##kbits##_newctx },                               \
    { Otls_FUNC_CIPHER_FREECTX, (void (*)(void)) alg##_freectx },              \
    { Otls_FUNC_CIPHER_DUPCTX, (void (*)(void)) alg##_dupctx },                \
    { Otls_FUNC_CIPHER_ENCRYPT_INIT, (void (*)(void))cipher_generic_einit },   \
    { Otls_FUNC_CIPHER_DECRYPT_INIT, (void (*)(void))cipher_generic_dinit },   \
    { Otls_FUNC_CIPHER_UPDATE, (void (*)(void))cipher_generic_##typ##_update },\
    { Otls_FUNC_CIPHER_FINAL, (void (*)(void))cipher_generic_##typ##_final },  \
    { Otls_FUNC_CIPHER_CIPHER, (void (*)(void))cipher_generic_cipher },        \
    { Otls_FUNC_CIPHER_GET_PARAMS,                                             \
      (void (*)(void)) alg##_##kbits##_get_params },                           \
    { Otls_FUNC_CIPHER_GET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_generic_get_ctx_params },                         \
    { Otls_FUNC_CIPHER_SET_CTX_PARAMS,                                         \
      (void (*)(void))cipher_var_keylen_set_ctx_params },                      \
    { Otls_FUNC_CIPHER_GETTABLE_PARAMS,                                        \
      (void (*)(void))cipher_generic_gettable_params },                        \
    { Otls_FUNC_CIPHER_GETTABLE_CTX_PARAMS,                                    \
      (void (*)(void))cipher_generic_gettable_ctx_params },                    \
    { Otls_FUNC_CIPHER_SETTABLE_CTX_PARAMS,                                    \
     (void (*)(void))cipher_var_keylen_settable_ctx_params },                  \
    { 0, NULL }                                                                \
};

/* rc440_functions */
IMPLEMENT_cipher(rc4, RC4, EVP_CIPH_VARIABLE_LENGTH, 40, 8, 0, stream)
/* rc4128_functions */
IMPLEMENT_cipher(rc4, RC4, EVP_CIPH_VARIABLE_LENGTH, 128, 8, 0, stream)
