/*
 * Copyright 1995-2016 The Opentls Project Authors. All Rights Reserved.
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
#include <opentls/x509.h>

X509_INFO *X509_INFO_new(void)
{
    X509_INFO *ret;

    ret = OPENtls_zalloc(sizeof(*ret));
    if (ret == NULL) {
        ASN1err(ASN1_F_X509_INFO_NEW, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    return ret;
}

void X509_INFO_free(X509_INFO *x)
{
    if (x == NULL)
        return;

    X509_free(x->x509);
    X509_CRL_free(x->crl);
    X509_PKEY_free(x->x_pkey);
    OPENtls_free(x->enc_data);
    OPENtls_free(x);
}
