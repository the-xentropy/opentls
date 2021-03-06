/*
 * Copyright 2019 The Opentls Project Authors. All Rights Reserved.
 * Copyright (c) 2019, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <string.h>
#include <opentls/err.h>
#include <opentls/lhash.h>
#include "internal/propertyerr.h"
#include "internal/property.h"
#include "property_local.h"

/*
 * Implement a property definition cache.
 * These functions assume that they are called under a write lock.
 * No attempt is made to clean out the cache, except when it is shut down.
 */

typedef struct {
    const char *prop;
    Otls_PROPERTY_LIST *defn;
    char body[1];
} PROPERTY_DEFN_ELEM;

DEFINE_LHASH_OF(PROPERTY_DEFN_ELEM);

static unsigned long property_defn_hash(const PROPERTY_DEFN_ELEM *a)
{
    return OPENtls_LH_strhash(a->prop);
}

static int property_defn_cmp(const PROPERTY_DEFN_ELEM *a,
                             const PROPERTY_DEFN_ELEM *b)
{
    return strcmp(a->prop, b->prop);
}

static void property_defn_free(PROPERTY_DEFN_ELEM *elem)
{
    otls_property_free(elem->defn);
    OPENtls_free(elem);
}

static void property_defns_free(void *vproperty_defns)
{
    LHASH_OF(PROPERTY_DEFN_ELEM) *property_defns = vproperty_defns;

    if (property_defns != NULL) {
        lh_PROPERTY_DEFN_ELEM_doall(property_defns,
                                    &property_defn_free);
        lh_PROPERTY_DEFN_ELEM_free(property_defns);
    }
}

static void *property_defns_new(OPENtls_CTX *ctx) {
    return lh_PROPERTY_DEFN_ELEM_new(&property_defn_hash, &property_defn_cmp);
}

static const OPENtls_CTX_METHOD property_defns_method = {
    property_defns_new,
    property_defns_free,
};

Otls_PROPERTY_LIST *otls_prop_defn_get(OPENtls_CTX *ctx, const char *prop)
{
    PROPERTY_DEFN_ELEM elem, *r;
    LHASH_OF(PROPERTY_DEFN_ELEM) *property_defns;

    property_defns = opentls_ctx_get_data(ctx, OPENtls_CTX_PROPERTY_DEFN_INDEX,
                                          &property_defns_method);
    if (property_defns == NULL)
        return NULL;

    elem.prop = prop;
    r = lh_PROPERTY_DEFN_ELEM_retrieve(property_defns, &elem);
    return r != NULL ? r->defn : NULL;
}

int otls_prop_defn_set(OPENtls_CTX *ctx, const char *prop,
                       Otls_PROPERTY_LIST *pl)
{
    PROPERTY_DEFN_ELEM elem, *old, *p = NULL;
    size_t len;
    LHASH_OF(PROPERTY_DEFN_ELEM) *property_defns;

    property_defns = opentls_ctx_get_data(ctx, OPENtls_CTX_PROPERTY_DEFN_INDEX,
                                          &property_defns_method);
    if (property_defns == NULL)
        return 0;

    if (prop == NULL)
        return 1;

    if (pl == NULL) {
        elem.prop = prop;
        lh_PROPERTY_DEFN_ELEM_delete(property_defns, &elem);
        return 1;
    }
    len = strlen(prop);
    p = OPENtls_malloc(sizeof(*p) + len);
    if (p != NULL) {
        p->prop = p->body;
        p->defn = pl;
        memcpy(p->body, prop, len + 1);
        old = lh_PROPERTY_DEFN_ELEM_insert(property_defns, p);
        if (old != NULL) {
            property_defn_free(old);
            return 1;
        }
        if (!lh_PROPERTY_DEFN_ELEM_error(property_defns))
            return 1;
    }
    OPENtls_free(p);
    return 0;
}
