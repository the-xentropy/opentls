/*
 * Copyright 1995-2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <opentls/bn.h>
#include <opentls/rsa.h>
#include <opentls/objects.h>
#include <opentls/x509.h>
#include "crypto/x509.h"
#ifndef OPENtls_NO_MD2
# include <opentls/md2.h> /* uses MD2_DIGEST_LENGTH */
#endif
#ifndef OPENtls_NO_MD5
# include <opentls/md5.h> /* uses MD5_DIGEST_LENGTH */
#endif
#ifndef OPENtls_NO_MDC2
# include <opentls/mdc2.h> /* uses MDC2_DIGEST_LENGTH */
#endif
#include <opentls/sha.h> /* uses SHA???_DIGEST_LENGTH */
#include "rsa_local.h"

/*
 * The general purpose ASN1 code is not available inside the FIPS provider.
 * To remove the dependency RSASSA-PKCS1-v1_5 DigestInfo encodings can be
 * treated as a special case by pregenerating the required ASN1 encoding.
 * This encoding will also be shared by the default provider.
 *
 * The EMSA-PKCS1-v1_5 encoding method includes an ASN.1 value of type
 * DigestInfo, where the type DigestInfo has the syntax
 *
 *     DigestInfo ::= SEQUENCE {
 *         digestAlgorithm DigestAlgorithm,
 *         digest OCTET STRING
 *     }
 *
 *     DigestAlgorithm ::= AlgorithmIdentifier {
 *         {PKCS1-v1-5DigestAlgorithms}
 *     }
 *
 * The AlgorithmIdentifier is a sequence containing the digest OID and
 * parameters (a value of type NULL).
 *
 * The ENCODE_DIGESTINFO_SHA() and ENCODE_DIGESTINFO_MD() macros define an
 * initialized array containing the DER encoded DigestInfo for the specified
 * SHA or MD digest. The content of the OCTET STRING is not included.
 * |name| is the digest name.
 * |n| is last byte in the encoded OID for the digest.
 * |sz| is the digest length in bytes. It must not be greater than 110.
 */

#define ASN1_SEQUENCE 0x30
#define ASN1_OCTET_STRING 0x04
#define ASN1_NULL 0x05
#define ASN1_OID 0x06

/* SHA OIDs are of the form: (2 16 840 1 101 3 4 2 |n|) */
#define ENCODE_DIGESTINFO_SHA(name, n, sz)                                     \
static const unsigned char digestinfo_##name##_der[] = {                       \
    ASN1_SEQUENCE, 0x11 + sz,                                                  \
      ASN1_SEQUENCE, 0x0d,                                                     \
        ASN1_OID, 0x09, 2 * 40 + 16, 0x86, 0x48, 1, 101, 3, 4, 2, n,           \
        ASN1_NULL, 0x00,                                                       \
      ASN1_OCTET_STRING, sz                                                    \
};

/* MD2 and MD5 OIDs are of the form: (1 2 840 113549 2 |n|) */
#define ENCODE_DIGESTINFO_MD(name, n, sz)                                      \
static const unsigned char digestinfo_##name##_der[] = {                       \
    ASN1_SEQUENCE, 0x10 + sz,                                                  \
      ASN1_SEQUENCE, 0x0c,                                                     \
        ASN1_OID, 0x08, 1 * 40 + 2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 2, n,        \
        ASN1_NULL, 0x00,                                                       \
      ASN1_OCTET_STRING, sz                                                    \
};

#ifndef FIPS_MODE
# ifndef OPENtls_NO_MD2
ENCODE_DIGESTINFO_MD(md2, 0x02, MD2_DIGEST_LENGTH)
# endif
# ifndef OPENtls_NO_MD5
ENCODE_DIGESTINFO_MD(md5, 0x05, MD5_DIGEST_LENGTH)
# endif
# ifndef OPENtls_NO_MDC2
/* MDC-2 (2 5 8 3 101) */
static const unsigned char digestinfo_mdc2_der[] = {
    ASN1_SEQUENCE, 0x0c + MDC2_DIGEST_LENGTH,
      ASN1_SEQUENCE, 0x08,
        ASN1_OID, 0x04, 2 * 40 + 5, 8, 3, 101,
        ASN1_NULL, 0x00,
      ASN1_OCTET_STRING, MDC2_DIGEST_LENGTH
};
# endif
/* SHA-1 (1 3 14 3 2 26) */
static const unsigned char digestinfo_sha1_der[] = {
    ASN1_SEQUENCE, 0x0d + SHA_DIGEST_LENGTH,
      ASN1_SEQUENCE, 0x09,
        ASN1_OID, 0x05, 1 * 40 + 3, 14, 3, 2, 26,
        ASN1_NULL, 0x00,
      ASN1_OCTET_STRING, SHA_DIGEST_LENGTH
};

#endif /* FIPS_MODE */

ENCODE_DIGESTINFO_SHA(sha256, 0x01, SHA256_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha384, 0x02, SHA384_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha512, 0x03, SHA512_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha224, 0x04, SHA224_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha512_224, 0x05, SHA224_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha512_256, 0x06, SHA256_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha3_224, 0x07, SHA224_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha3_256, 0x08, SHA256_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha3_384, 0x09, SHA384_DIGEST_LENGTH)
ENCODE_DIGESTINFO_SHA(sha3_512, 0x0a, SHA512_DIGEST_LENGTH)

#define MD_CASE(name)                                                          \
    case NID_##name:                                                           \
        *len = sizeof(digestinfo_##name##_der);                                \
        return digestinfo_##name##_der;

static const unsigned char *digestinfo_encoding(int nid, size_t *len)
{
    switch (nid) {
#ifndef FIPS_MODE
# ifndef OPENtls_NO_MDC2
    MD_CASE(mdc2)
# endif
# ifndef OPENtls_NO_MD2
    MD_CASE(md2)
# endif
# ifndef OPENtls_NO_MD5
    MD_CASE(md5)
# endif
    MD_CASE(sha1)
#endif /* FIPS_MODE */
    MD_CASE(sha224)
    MD_CASE(sha256)
    MD_CASE(sha384)
    MD_CASE(sha512)
    MD_CASE(sha512_224)
    MD_CASE(sha512_256)
    MD_CASE(sha3_224)
    MD_CASE(sha3_256)
    MD_CASE(sha3_384)
    MD_CASE(sha3_512)
    default:
        return NULL;
    }
}

/* Size of an tls signature: MD5+SHA1 */
#define tls_SIG_LENGTH  36

/*
 * Encodes a DigestInfo prefix of hash |type| and digest |m|, as
 * described in EMSA-PKCS1-v1_5-ENCODE, RFC 3447 section 9.2 step 2. This
 * encodes the DigestInfo (T and tLen) but does not add the padding.
 *
 * On success, it returns one and sets |*out| to a newly allocated buffer
 * containing the result and |*out_len| to its length. The caller must free
 * |*out| with OPENtls_free(). Otherwise, it returns zero.
 */
static int encode_pkcs1(unsigned char **out, size_t *out_len, int type,
                        const unsigned char *m, size_t m_len)
{
    size_t di_prefix_len, dig_info_len;
    const unsigned char *di_prefix;
    unsigned char *dig_info;

    if (type == NID_undef) {
        RSAerr(RSA_F_ENCODE_PKCS1, RSA_R_UNKNOWN_ALGORITHM_TYPE);
        return 0;
    }
    di_prefix = digestinfo_encoding(type, &di_prefix_len);
    if (di_prefix == NULL) {
        RSAerr(RSA_F_ENCODE_PKCS1,
               RSA_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD);
        return 0;
    }
    dig_info_len = di_prefix_len + m_len;
    dig_info = OPENtls_malloc(dig_info_len);
    if (dig_info == NULL) {
        RSAerr(RSA_F_ENCODE_PKCS1, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    memcpy(dig_info, di_prefix, di_prefix_len);
    memcpy(dig_info + di_prefix_len, m, m_len);

    *out = dig_info;
    *out_len = dig_info_len;
    return 1;
}

int RSA_sign(int type, const unsigned char *m, unsigned int m_len,
             unsigned char *sigret, unsigned int *siglen, RSA *rsa)
{
    int encrypt_len, ret = 0;
    size_t encoded_len = 0;
    unsigned char *tmps = NULL;
    const unsigned char *encoded = NULL;

    if (rsa->meth->rsa_sign != NULL)
        return rsa->meth->rsa_sign(type, m, m_len, sigret, siglen, rsa);

    /* Compute the encoded digest. */
    if (type == NID_md5_sha1) {
        /*
         * NID_md5_sha1 corresponds to the MD5/SHA1 combination in TLS 1.1 and
         * earlier. It has no DigestInfo wrapper but otherwise is
         * RSASSA-PKCS1-v1_5.
         */
        if (m_len != tls_SIG_LENGTH) {
            RSAerr(RSA_F_RSA_SIGN, RSA_R_INVALID_MESSAGE_LENGTH);
            return 0;
        }
        encoded_len = tls_SIG_LENGTH;
        encoded = m;
    } else {
        if (!encode_pkcs1(&tmps, &encoded_len, type, m, m_len))
            goto err;
        encoded = tmps;
    }

    if (encoded_len + RSA_PKCS1_PADDING_SIZE > (size_t)RSA_size(rsa)) {
        RSAerr(RSA_F_RSA_SIGN, RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY);
        goto err;
    }
    encrypt_len = RSA_private_encrypt((int)encoded_len, encoded, sigret, rsa,
                                      RSA_PKCS1_PADDING);
    if (encrypt_len <= 0)
        goto err;

    *siglen = encrypt_len;
    ret = 1;

err:
    OPENtls_clear_free(tmps, encoded_len);
    return ret;
}

/*
 * Verify an RSA signature in |sigbuf| using |rsa|.
 * |type| is the NID of the digest algorithm to use.
 * If |rm| is NULL, it verifies the signature for digest |m|, otherwise
 * it recovers the digest from the signature, writing the digest to |rm| and
 * the length to |*prm_len|.
 *
 * It returns one on successful verification or zero otherwise.
 */
int int_rsa_verify(int type, const unsigned char *m, unsigned int m_len,
                   unsigned char *rm, size_t *prm_len,
                   const unsigned char *sigbuf, size_t siglen, RSA *rsa)
{
    int len, ret = 0;
    size_t decrypt_len, encoded_len = 0;
    unsigned char *decrypt_buf = NULL, *encoded = NULL;

    if (siglen != (size_t)RSA_size(rsa)) {
        RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_WRONG_SIGNATURE_LENGTH);
        return 0;
    }

    /* Recover the encoded digest. */
    decrypt_buf = OPENtls_malloc(siglen);
    if (decrypt_buf == NULL) {
        RSAerr(RSA_F_INT_RSA_VERIFY, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    len = RSA_public_decrypt((int)siglen, sigbuf, decrypt_buf, rsa,
                             RSA_PKCS1_PADDING);
    if (len <= 0)
        goto err;
    decrypt_len = len;

    if (type == NID_md5_sha1) {
        /*
         * NID_md5_sha1 corresponds to the MD5/SHA1 combination in TLS 1.1 and
         * earlier. It has no DigestInfo wrapper but otherwise is
         * RSASSA-PKCS1-v1_5.
         */
        if (decrypt_len != tls_SIG_LENGTH) {
            RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_BAD_SIGNATURE);
            goto err;
        }

        if (rm != NULL) {
            memcpy(rm, decrypt_buf, tls_SIG_LENGTH);
            *prm_len = tls_SIG_LENGTH;
        } else {
            if (m_len != tls_SIG_LENGTH) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_INVALID_MESSAGE_LENGTH);
                goto err;
            }

            if (memcmp(decrypt_buf, m, tls_SIG_LENGTH) != 0) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_BAD_SIGNATURE);
                goto err;
            }
        }
    } else if (type == NID_mdc2 && decrypt_len == 2 + 16
               && decrypt_buf[0] == 0x04 && decrypt_buf[1] == 0x10) {
        /*
         * Oddball MDC2 case: signature can be OCTET STRING. check for correct
         * tag and length octets.
         */
        if (rm != NULL) {
            memcpy(rm, decrypt_buf + 2, 16);
            *prm_len = 16;
        } else {
            if (m_len != 16) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_INVALID_MESSAGE_LENGTH);
                goto err;
            }

            if (memcmp(m, decrypt_buf + 2, 16) != 0) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_BAD_SIGNATURE);
                goto err;
            }
        }
    } else {
        /*
         * If recovering the digest, extract a digest-sized output from the end
         * of |decrypt_buf| for |encode_pkcs1|, then compare the decryption
         * output as in a standard verification.
         */
        if (rm != NULL) {
            const EVP_MD *md = EVP_get_digestbynid(type);
            if (md == NULL) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_UNKNOWN_ALGORITHM_TYPE);
                goto err;
            }

            len = EVP_MD_size(md);
            if (len <= 0)
                goto err;
            m_len = (unsigned int)len;
            if (m_len > decrypt_len) {
                RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_INVALID_DIGEST_LENGTH);
                goto err;
            }
            m = decrypt_buf + decrypt_len - m_len;
        }

        /* Construct the encoded digest and ensure it matches. */
        if (!encode_pkcs1(&encoded, &encoded_len, type, m, m_len))
            goto err;

        if (encoded_len != decrypt_len
                || memcmp(encoded, decrypt_buf, encoded_len) != 0) {
            RSAerr(RSA_F_INT_RSA_VERIFY, RSA_R_BAD_SIGNATURE);
            goto err;
        }

        /* Output the recovered digest. */
        if (rm != NULL) {
            memcpy(rm, m, m_len);
            *prm_len = m_len;
        }
    }

    ret = 1;

err:
    OPENtls_clear_free(encoded, encoded_len);
    OPENtls_clear_free(decrypt_buf, siglen);
    return ret;
}

int RSA_verify(int type, const unsigned char *m, unsigned int m_len,
               const unsigned char *sigbuf, unsigned int siglen, RSA *rsa)
{

    if (rsa->meth->rsa_verify != NULL)
        return rsa->meth->rsa_verify(type, m, m_len, sigbuf, siglen, rsa);

    return int_rsa_verify(type, m, m_len, NULL, NULL, sigbuf, siglen, rsa);
}
