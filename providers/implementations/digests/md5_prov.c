/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <opentls/crypto.h>
#include <opentls/md5.h>
#include "prov/digestcommon.h"
#include "prov/implementations.h"

/* md5_functions */
IMPLEMENT_digest_functions(md5, MD5_CTX,
                           MD5_CBLOCK, MD5_DIGEST_LENGTH, 0,
                           MD5_Init, MD5_Update, MD5_Final)
