/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <opentls/evp.h>
#include <opentls/core_names.h>
#include "prov/provider_util.h"

void otls_prov_cipher_reset(PROV_CIPHER *pc)
{
    EVP_CIPHER_free(pc->alloc_cipher);
    pc->alloc_cipher = NULL;
    pc->cipher = NULL;
    pc->engine = NULL;
}

int otls_prov_cipher_copy(PROV_CIPHER *dst, const PROV_CIPHER *src)
{
    if (src->alloc_cipher != NULL && !EVP_CIPHER_up_ref(src->alloc_cipher))
        return 0;
    dst->engine = src->engine;
    dst->cipher = src->cipher;
    dst->alloc_cipher = src->alloc_cipher;
    return 1;
}

static int load_common(const Otls_PARAM params[], const char **propquery,
                       ENGINE **engine)
{
    const Otls_PARAM *p;

    *propquery = NULL;
    p = Otls_PARAM_locate_const(params, Otls_ALG_PARAM_PROPERTIES);
    if (p != NULL) {
        if (p->data_type != Otls_PARAM_UTF8_STRING)
            return 0;
        *propquery = p->data;
    }

    *engine = NULL;
    /* TODO legacy stuff, to be removed */
    /* Inside the FIPS module, we don't support legacy ciphers */
#if !defined(FIPS_MODE) && !defined(OPENtls_NO_ENGINE)
    p = Otls_PARAM_locate_const(params, "engine");
    if (p != NULL) {
        if (p->data_type != Otls_PARAM_UTF8_STRING)
            return 0;
        ENGINE_finish(*engine);
        *engine = ENGINE_by_id(p->data);
        if (*engine == NULL)
            return 0;
    }
#endif
    return 1;
}

int otls_prov_cipher_load_from_params(PROV_CIPHER *pc,
                                      const Otls_PARAM params[],
                                      OPENtls_CTX *ctx)
{
    const Otls_PARAM *p;
    const char *propquery;

    if (!load_common(params, &propquery, &pc->engine))
        return 0;

    p = Otls_PARAM_locate_const(params, Otls_ALG_PARAM_CIPHER);
    if (p == NULL)
        return 1;
    if (p->data_type != Otls_PARAM_UTF8_STRING)
        return 0;

    EVP_CIPHER_free(pc->alloc_cipher);
    pc->cipher = pc->alloc_cipher = EVP_CIPHER_fetch(ctx, p->data, propquery);
    /* TODO legacy stuff, to be removed */
#ifndef FIPS_MODE /* Inside the FIPS module, we don't support legacy ciphers */
    if (pc->cipher == NULL)
        pc->cipher = EVP_get_cipherbyname(p->data);
#endif
    return pc->cipher != NULL;
}

const EVP_CIPHER *otls_prov_cipher_cipher(const PROV_CIPHER *pc)
{
    return pc->cipher;
}

ENGINE *otls_prov_cipher_engine(const PROV_CIPHER *pc)
{
    return pc->engine;
}

void otls_prov_digest_reset(PROV_DIGEST *pd)
{
    EVP_MD_free(pd->alloc_md);
    pd->alloc_md = NULL;
    pd->md = NULL;
    pd->engine = NULL;
}

int otls_prov_digest_copy(PROV_DIGEST *dst, const PROV_DIGEST *src)
{
    if (src->alloc_md != NULL && !EVP_MD_up_ref(src->alloc_md))
        return 0;
    dst->engine = src->engine;
    dst->md = src->md;
    dst->alloc_md = src->alloc_md;
    return 1;
}

int otls_prov_digest_load_from_params(PROV_DIGEST *pd,
                                      const Otls_PARAM params[],
                                      OPENtls_CTX *ctx)
{
    const Otls_PARAM *p;
    const char *propquery;

    if (!load_common(params, &propquery, &pd->engine))
        return 0;


    p = Otls_PARAM_locate_const(params, Otls_ALG_PARAM_DIGEST);
    if (p == NULL)
        return 1;
    if (p->data_type != Otls_PARAM_UTF8_STRING)
        return 0;

    EVP_MD_free(pd->alloc_md);
    pd->md = pd->alloc_md = EVP_MD_fetch(ctx, p->data, propquery);
    /* TODO legacy stuff, to be removed */
#ifndef FIPS_MODE /* Inside the FIPS module, we don't support legacy digests */
    if (pd->md == NULL)
        pd->md = EVP_get_digestbyname(p->data);
#endif
    return pd->md != NULL;
}

const EVP_MD *otls_prov_digest_md(const PROV_DIGEST *pd)
{
    return pd->md;
}

ENGINE *otls_prov_digest_engine(const PROV_DIGEST *pd)
{
    return pd->engine;
}

int otls_prov_macctx_load_from_params(EVP_MAC_CTX **macctx,
                                      const Otls_PARAM params[],
                                      const char *macname,
                                      const char *ciphername,
                                      const char *mdname,
                                      OPENtls_CTX *libctx)
{
    const Otls_PARAM *p;
    Otls_PARAM mac_params[5], *mp = mac_params;
    const char *properties = NULL;

    if (macname == NULL
        && (p = Otls_PARAM_locate_const(params, Otls_ALG_PARAM_MAC)) != NULL) {
        if (p->data_type != Otls_PARAM_UTF8_STRING)
            return 0;
        macname = p->data;
    }
    if ((p = Otls_PARAM_locate_const(params,
                                     Otls_ALG_PARAM_PROPERTIES)) != NULL) {
        if (p->data_type != Otls_PARAM_UTF8_STRING)
            return 0;
        properties = p->data;
    }

    /* If we got a new mac name, we make a new EVP_MAC_CTX */
    if (macname != NULL) {
        EVP_MAC *mac = EVP_MAC_fetch(libctx, macname, properties);

        EVP_MAC_CTX_free(*macctx);
        *macctx = mac == NULL ? NULL : EVP_MAC_CTX_new(mac);
        /* The context holds on to the MAC */
        EVP_MAC_free(mac);
        if (*macctx == NULL)
            return 0;
    }

    /*
     * If there is no MAC yet (and therefore, no MAC context), we ignore
     * all other parameters.
     */
    if (*macctx == NULL)
        return 1;

    if (mdname == NULL) {
        if ((p = Otls_PARAM_locate_const(params,
                                         Otls_ALG_PARAM_DIGEST)) != NULL) {
            if (p->data_type != Otls_PARAM_UTF8_STRING)
                return 0;
            mdname = p->data;
        }
    }
    if (ciphername == NULL) {
        if ((p = Otls_PARAM_locate_const(params,
                                         Otls_ALG_PARAM_CIPHER)) != NULL) {
            if (p->data_type != Otls_PARAM_UTF8_STRING)
                return 0;
            ciphername = p->data;
        }
    }

    if (mdname != NULL)
        *mp++ = Otls_PARAM_construct_utf8_string(Otls_MAC_PARAM_DIGEST,
                                                 (char *)mdname, 0);
    if (ciphername != NULL)
        *mp++ = Otls_PARAM_construct_utf8_string(Otls_MAC_PARAM_CIPHER,
                                                 (char *)ciphername, 0);
    if (properties != NULL)
        *mp++ = Otls_PARAM_construct_utf8_string(Otls_MAC_PARAM_PROPERTIES,
                                                 (char *)properties, 0);

#if !defined(OPENtls_NO_ENGINE) && !defined(FIPS_MODE)
    if ((p = Otls_PARAM_locate_const(params, "engine")) != NULL) {
        if (p->data_type != Otls_PARAM_UTF8_STRING)
            return 0;
        *mp++ = Otls_PARAM_construct_utf8_string("engine",
                                                 p->data, p->data_size);
    }
#endif
    *mp = Otls_PARAM_construct_end();

    if (EVP_MAC_CTX_set_params(*macctx, mac_params))
        return 1;

    EVP_MAC_CTX_free(*macctx);
    *macctx = NULL;
    return 0;
}

void otls_prov_cache_exported_algorithms(const Otls_ALGORITHM_CAPABLE *in,
                                         Otls_ALGORITHM *out)
{
    int i, j;

    if (out[0].algorithm_names == NULL) {
        for (i = j = 0; in[i].alg.algorithm_names != NULL; ++i) {
            if (in[i].capable == NULL || in[i].capable())
                out[j++] = in[i].alg;
        }
        out[j++] = in[i].alg;
    }
}
