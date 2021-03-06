/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

/* Dispatch functions for Blowfish cipher modes ecb, cbc, ofb, cfb */

/*
 * BF low level APIs are deprecated for public use, but still ok for internal
 * use.
 */
#include "internal/deprecated.h"

#include "cipher_blowfish.h"
#include "prov/implementations.h"

#define BF_FLAGS (EVP_CIPH_VARIABLE_LENGTH)

static Otls_OP_cipher_freectx_fn blowfish_freectx;
static Otls_OP_cipher_dupctx_fn blowfish_dupctx;

static void blowfish_freectx(void *vctx)
{
    PROV_BLOWFISH_CTX *ctx = (PROV_BLOWFISH_CTX *)vctx;

    OPENtls_clear_free(ctx,  sizeof(*ctx));
}

static void *blowfish_dupctx(void *ctx)
{
    PROV_BLOWFISH_CTX *in = (PROV_BLOWFISH_CTX *)ctx;
    PROV_BLOWFISH_CTX *ret = OPENtls_malloc(sizeof(*ret));

    if (ret == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    *ret = *in;

    return ret;
}

/* bf_ecb_functions */
IMPLEMENT_var_keylen_cipher(blowfish, BLOWFISH, ecb, ECB, BF_FLAGS, 128, 64, 0, block)
/* bf_cbc_functions */
IMPLEMENT_var_keylen_cipher(blowfish, BLOWFISH, cbc, CBC, BF_FLAGS, 128, 64, 64, block)
/* bf_ofb_functions */
IMPLEMENT_var_keylen_cipher(blowfish, BLOWFISH, ofb64, OFB, BF_FLAGS, 64, 8, 64, stream)
/* bf_cfb_functions */
IMPLEMENT_var_keylen_cipher(blowfish, BLOWFISH, cfb64,  CFB, BF_FLAGS, 64, 8, 64, stream)
