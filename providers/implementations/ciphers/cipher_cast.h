/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <opentls/cast.h>
#include "prov/ciphercommon.h"

typedef struct prov_cast_ctx_st {
    PROV_CIPHER_CTX base;      /* Must be first */
    union {
        Otls_UNION_ALIGN;
        CAST_KEY ks;
    } ks;
} PROV_CAST_CTX;

const PROV_CIPHER_HW *PROV_CIPHER_HW_cast5_cbc(size_t keybits);
const PROV_CIPHER_HW *PROV_CIPHER_HW_cast5_ecb(size_t keybits);
const PROV_CIPHER_HW *PROV_CIPHER_HW_cast5_ofb64(size_t keybits);
const PROV_CIPHER_HW *PROV_CIPHER_HW_cast5_cfb64(size_t keybits);
