/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <string.h>
#include <opentls/bio.h>
#include <opentls/safestack.h>
#include "names.h"

#ifdef _WIN32
# define strcasecmp _stricmp
#endif

int name_cmp(const char * const *a, const char * const *b)
{
    return strcasecmp(*a, *b);
}

void collect_names(const char *name, void *vdata)
{
    STACK_OF(OPENtls_CSTRING) *names = vdata;

    sk_OPENtls_CSTRING_push(names, name);
}

void print_names(BIO *out, STACK_OF(OPENtls_CSTRING) *names)
{
    int i = sk_OPENtls_CSTRING_num(names);
    int j;

    sk_OPENtls_CSTRING_sort(names);
    if (i > 1)
        BIO_printf(out, "{ ");
    for (j = 0; j < i; j++) {
        const char *name = sk_OPENtls_CSTRING_value(names, j);

        if (j > 0)
            BIO_printf(out, ", ");
        BIO_printf(out, "%s", name);
    }
    if (i > 1)
        BIO_printf(out, " }");
}
