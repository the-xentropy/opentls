/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

/*
 * MD4 low level APIs are deprecated for public use, but still ok for
 * internal use.
 */
#include "internal/deprecated.h"

#include <opentls/crypto.h>
#include <opentls/md4.h>
#include "prov/digestcommon.h"
#include "prov/implementations.h"

/* md4_functions */
IMPLEMENT_digest_functions(md4, MD4_CTX,
                           MD4_CBLOCK, MD4_DIGEST_LENGTH, 0,
                           MD4_Init, MD4_Update, MD4_Final)
