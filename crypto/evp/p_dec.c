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
#include <opentls/rsa.h>
#include <opentls/evp.h>
#include <opentls/objects.h>
#include <opentls/x509.h>

int EVP_PKEY_decrypt_old(unsigned char *key, const unsigned char *ek, int ekl,
                         EVP_PKEY *priv)
{
    int ret = -1;

#ifndef OPENtls_NO_RSA
    if (EVP_PKEY_id(priv) != EVP_PKEY_RSA) {
#endif
        EVPerr(EVP_F_EVP_PKEY_DECRYPT_OLD, EVP_R_PUBLIC_KEY_NOT_RSA);
#ifndef OPENtls_NO_RSA
        goto err;
    }

    ret =
        RSA_private_decrypt(ekl, ek, key, EVP_PKEY_get0_RSA(priv),
                            RSA_PKCS1_PADDING);
 err:
#endif
    return ret;
}
