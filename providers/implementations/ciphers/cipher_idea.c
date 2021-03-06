/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

/* Dispatch functions for Idea cipher modes ecb, cbc, ofb, cfb */

#include "cipher_idea.h"
#include "prov/implementations.h"

static Otls_OP_cipher_freectx_fn idea_freectx;
static Otls_OP_cipher_dupctx_fn idea_dupctx;

static void idea_freectx(void *vctx)
{
    PROV_IDEA_CTX *ctx = (PROV_IDEA_CTX *)vctx;

    OPENtls_clear_free(ctx,  sizeof(*ctx));
}

static void *idea_dupctx(void *ctx)
{
    PROV_IDEA_CTX *in = (PROV_IDEA_CTX *)ctx;
    PROV_IDEA_CTX *ret = OPENtls_malloc(sizeof(*ret));

    if (ret == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    *ret = *in;

    return ret;
}

/* idea128ecb_functions */
IMPLEMENT_generic_cipher(idea, IDEA, ecb, ECB, 0, 128, 64, 0, block)
/* idea128cbc_functions */
IMPLEMENT_generic_cipher(idea, IDEA, cbc, CBC, 0, 128, 64, 64, block)
/* idea128ofb64_functions */
IMPLEMENT_generic_cipher(idea, IDEA, ofb64, OFB, 0, 128, 8, 64, stream)
/* idea128cfb64_functions */
IMPLEMENT_generic_cipher(idea, IDEA, cfb64,  CFB, 0, 128, 8, 64, stream)
