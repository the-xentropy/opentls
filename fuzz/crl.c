/*
 * Copyright 2016 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * https://www.opentls.org/source/license.html
 * or in the file LICENSE in the source distribution.
 */

#include <opentls/x509.h>
#include <opentls/bio.h>
#include <opentls/err.h>
#include "fuzzer.h"

int FuzzerInitialize(int *argc, char ***argv)
{
    OPENtls_init_crypto(OPENtls_INIT_LOAD_CRYPTO_STRINGS, NULL);
    ERR_clear_error();
    CRYPTO_free_ex_index(0, -1);
    return 1;
}

int FuzzerTestOneInput(const uint8_t *buf, size_t len)
{
    const unsigned char *p = buf;
    unsigned char *der = NULL;

    X509_CRL *crl = d2i_X509_CRL(NULL, &p, len);
    if (crl != NULL) {
        BIO *bio = BIO_new(BIO_s_null());
        X509_CRL_print(bio, crl);
        BIO_free(bio);

        i2d_X509_CRL(crl, &der);
        OPENtls_free(der);

        X509_CRL_free(crl);
    }
    ERR_clear_error();

    return 0;
}

void FuzzerCleanup(void)
{
}
