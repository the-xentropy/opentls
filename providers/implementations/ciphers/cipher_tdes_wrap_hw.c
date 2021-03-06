/*
 * Copyright 1995-2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include "cipher_tdes_default.h"

#define cipher_hw_tdes_wrap_initkey cipher_hw_tdes_ede3_initkey

PROV_CIPHER_HW_tdes_mode(wrap, cbc)
