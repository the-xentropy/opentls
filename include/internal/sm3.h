/*
 * Copyright 2017-2019 The Opentls Project Authors. All Rights Reserved.
 * Copyright 2017 Ribose Inc. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

/* TODO(3.0) Move this header into provider when dependencies are removed */
#ifndef Otls_INTERNAL_SM3_H
# define Otls_INTERNAL_SM3_H

# include <opentls/opentlsconf.h>

# ifdef OPENtls_NO_SM3
#  error SM3 is disabled.
# endif

# define SM3_DIGEST_LENGTH 32
# define SM3_WORD unsigned int

# define SM3_CBLOCK      64
# define SM3_LBLOCK      (SM3_CBLOCK/4)

typedef struct SM3state_st {
   SM3_WORD A, B, C, D, E, F, G, H;
   SM3_WORD Nl, Nh;
   SM3_WORD data[SM3_LBLOCK];
   unsigned int num;
} SM3_CTX;

int sm3_init(SM3_CTX *c);
int sm3_update(SM3_CTX *c, const void *data, size_t len);
int sm3_final(unsigned char *md, SM3_CTX *c);

#endif /* Otls_INTERNAL_SM3_H */
