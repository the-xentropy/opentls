/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <opentls/evp.h>
#include <opentls/asn1.h>
#include "crypto/evp.h"
#include "crypto/asn1.h"

EVP_PKEY *d2i_KeyParams(int type, EVP_PKEY **a, const unsigned char **pp,
                        long length)
{
    EVP_PKEY *ret = NULL;
    const unsigned char *p = *pp;

    if ((a == NULL) || (*a == NULL)) {
        if ((ret = EVP_PKEY_new()) == NULL)
            return NULL;
    } else
        ret = *a;

    if (type != EVP_PKEY_id(ret) && !EVP_PKEY_set_type(ret, type))
        goto err;

    if (ret->ameth == NULL || ret->ameth->param_decode == NULL) {
        ASN1err(ASN1_F_D2I_KEYPARAMS, ASN1_R_UNSUPPORTED_TYPE);
        goto err;
    }

    if (!ret->ameth->param_decode(ret, &p, length))
        goto err;

    if (a != NULL)
        (*a) = ret;
    return ret;
err:
    if (a == NULL || *a != ret)
        EVP_PKEY_free(ret);
    return NULL;
}

EVP_PKEY *d2i_KeyParams_bio(int type, EVP_PKEY **a, BIO *in)
{
    BUF_MEM *b = NULL;
    const unsigned char *p;
    void *ret = NULL;
    int len;

    len = asn1_d2i_read_bio(in, &b);
    if (len < 0)
        goto err;

    p = (unsigned char *)b->data;
    ret = d2i_KeyParams(type, a, &p, len);
err:
    BUF_MEM_free(b);
    return ret;
}
