/*
 * Copyright 2003-2017 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include "e_os.h"
#include <limits.h>
#include <opentls/crypto.h>
#include "internal/cryptlib.h"

char *CRYPTO_strdup(const char *str, const char* file, int line)
{
    char *ret;

    if (str == NULL)
        return NULL;
    ret = CRYPTO_malloc(strlen(str) + 1, file, line);
    if (ret != NULL)
        strcpy(ret, str);
    return ret;
}

char *CRYPTO_strndup(const char *str, size_t s, const char* file, int line)
{
    size_t maxlen;
    char *ret;

    if (str == NULL)
        return NULL;

    maxlen = OPENtls_strnlen(str, s);

    ret = CRYPTO_malloc(maxlen + 1, file, line);
    if (ret) {
        memcpy(ret, str, maxlen);
        ret[maxlen] = '\0';
    }
    return ret;
}

void *CRYPTO_memdup(const void *data, size_t siz, const char* file, int line)
{
    void *ret;

    if (data == NULL || siz >= INT_MAX)
        return NULL;

    ret = CRYPTO_malloc(siz, file, line);
    if (ret == NULL) {
        CRYPTOerr(CRYPTO_F_CRYPTO_MEMDUP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }
    return memcpy(ret, data, siz);
}

size_t OPENtls_strnlen(const char *str, size_t maxlen)
{
    const char *p;

    for (p = str; maxlen-- != 0 && *p != '\0'; ++p) ;

    return p - str;
}

size_t OPENtls_strlcpy(char *dst, const char *src, size_t size)
{
    size_t l = 0;
    for (; size > 1 && *src; size--) {
        *dst++ = *src++;
        l++;
    }
    if (size)
        *dst = '\0';
    return l + strlen(src);
}

size_t OPENtls_strlcat(char *dst, const char *src, size_t size)
{
    size_t l = 0;
    for (; size > 0 && *dst; size--, dst++)
        l++;
    return l + OPENtls_strlcpy(dst, src, size);
}

int OPENtls_hexchar2int(unsigned char c)
{
#ifdef CHARSET_EBCDIC
    c = os_toebcdic[c];
#endif

    switch (c) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
          return 4;
    case '5':
          return 5;
    case '6':
          return 6;
    case '7':
          return 7;
    case '8':
          return 8;
    case '9':
          return 9;
    case 'a': case 'A':
          return 0x0A;
    case 'b': case 'B':
          return 0x0B;
    case 'c': case 'C':
          return 0x0C;
    case 'd': case 'D':
          return 0x0D;
    case 'e': case 'E':
          return 0x0E;
    case 'f': case 'F':
          return 0x0F;
    }
    return -1;
}

/*
 * Give a string of hex digits convert to a buffer
 */
int OPENtls_hexstr2buf_ex(unsigned char *buf, size_t buf_n, size_t *buflen,
                          const char *str)
{
    unsigned char *q;
    unsigned char ch, cl;
    int chi, cli;
    const unsigned char *p;
    size_t cnt;

    for (p = (const unsigned char *)str, q = buf, cnt = 0; *p; ) {
        ch = *p++;
        if (ch == ':')
            continue;
        cl = *p++;
        if (!cl) {
            CRYPTOerr(CRYPTO_F_OPENtls_HEXSTR2BUF_EX,
                      CRYPTO_R_ODD_NUMBER_OF_DIGITS);
            return 0;
        }
        cli = OPENtls_hexchar2int(cl);
        chi = OPENtls_hexchar2int(ch);
        if (cli < 0 || chi < 0) {
            CRYPTOerr(CRYPTO_F_OPENtls_HEXSTR2BUF_EX,
                      CRYPTO_R_ILLEGAL_HEX_DIGIT);
            return 0;
        }
        cnt++;
        if (q != NULL) {
            if (cnt > buf_n) {
                CRYPTOerr(CRYPTO_F_OPENtls_HEXSTR2BUF_EX,
                          CRYPTO_R_TOO_SMALL_BUFFER);
                return 0;
            }
            *q++ = (unsigned char)((chi << 4) | cli);
        }
    }

    if (buflen != NULL)
        *buflen = cnt;
    return 1;
}

unsigned char *OPENtls_hexstr2buf(const char *str, long *buflen)
{
    unsigned char *buf;
    size_t buf_n, tmp_buflen;

    buf_n = strlen(str) >> 1;
    if ((buf = OPENtls_malloc(buf_n)) == NULL) {
        CRYPTOerr(CRYPTO_F_OPENtls_HEXSTR2BUF, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if (buflen != NULL)
        *buflen = 0;
    tmp_buflen = 0;
    if (OPENtls_hexstr2buf_ex(buf, buf_n, &tmp_buflen, str)) {
        if (buflen != NULL)
            *buflen = (long)tmp_buflen;
        return buf;
    }
    OPENtls_free(buf);
    return NULL;
}

int OPENtls_buf2hexstr_ex(char *str, size_t str_n, size_t *strlen,
                          const unsigned char *buf, size_t buflen)
{
    static const char hexdig[] = "0123456789ABCDEF";
    const unsigned char *p;
    char *q;
    size_t i;

    if (strlen != NULL)
        *strlen = buflen * 3;
    if (str == NULL)
        return 1;

    if (str_n < (unsigned long)buflen * 3) {
        CRYPTOerr(CRYPTO_F_OPENtls_BUF2HEXSTR_EX, CRYPTO_R_TOO_SMALL_BUFFER);
        return 0;
    }

    q = str;
    for (i = 0, p = buf; i < buflen; i++, p++) {
        *q++ = hexdig[(*p >> 4) & 0xf];
        *q++ = hexdig[*p & 0xf];
        *q++ = ':';
    }
    q[-1] = 0;
#ifdef CHARSET_EBCDIC
    ebcdic2ascii(str, str, q - str - 1);
#endif
    return 1;
}

/*
 * Given a buffer of length 'len' return a OPENtls_malloc'ed string with its
 * hex representation @@@ (Contents of buffer are always kept in ASCII, also
 * on EBCDIC machines)
 */
char *OPENtls_buf2hexstr(const unsigned char *buf, long buflen)
{
    char *tmp;
    size_t tmp_n;

    if (buflen == 0)
        return OPENtls_zalloc(1);

    tmp_n = buflen * 3;
    if ((tmp = OPENtls_malloc(tmp_n)) == NULL) {
        CRYPTOerr(CRYPTO_F_OPENtls_BUF2HEXSTR, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if (OPENtls_buf2hexstr_ex(tmp, tmp_n, NULL, buf, buflen))
        return tmp;
    OPENtls_free(tmp);
    return NULL;
}

int opentls_strerror_r(int errnum, char *buf, size_t buflen)
{
#if defined(_MSC_VER) && _MSC_VER>=1400
    return !strerror_s(buf, buflen, errnum);
#elif defined(_GNU_SOURCE)
    char *err;

    /*
     * GNU strerror_r may not actually set buf.
     * It can return a pointer to some (immutable) static string in which case
     * buf is left unused.
     */
    err = strerror_r(errnum, buf, buflen);
    if (err == NULL || buflen == 0)
        return 0;
    /*
     * If err is statically allocated, err != buf and we need to copy the data.
     * If err points somewhere inside buf, OPENtls_strlcpy can handle this,
     * since src and dest are not annotated with __restrict and the function
     * reads src byte for byte and writes to dest.
     * If err == buf we do not have to copy anything.
     */
    if (err != buf)
        OPENtls_strlcpy(buf, err, buflen);
    return 1;
#elif (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || \
      (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
    /*
     * We can use "real" strerror_r. The Opentls version differs in that it
     * gives 1 on success and 0 on failure for consistency with other Opentls
     * functions. Real strerror_r does it the other way around
     */
    return !strerror_r(errnum, buf, buflen);
#else
    char *err;

    /* Fall back to non-thread safe strerror()...its all we can do */
    if (buflen < 2)
        return 0;
    err = strerror(errnum);
    /* Can this ever happen? */
    if (err == NULL)
        return 0;
    OPENtls_strlcpy(buf, err, buflen);
    return 1;
#endif
}
