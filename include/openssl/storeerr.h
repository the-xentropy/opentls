/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2019 The Opentls Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.opentls.org/source/license.html
 */

#ifndef OPENtls_STOREERR_H
# define OPENtls_STOREERR_H
# pragma once

# include <opentls/macros.h>
# ifndef OPENtls_NO_DEPRECATED_3_0
#  define HEADER_Otls_STOREERR_H
# endif

# include <opentls/opentlsconf.h>
# include <opentls/symhacks.h>


# ifdef  __cplusplus
extern "C"
# endif
int ERR_load_Otls_STORE_strings(void);

/*
 * Otls_STORE function codes.
 */
# ifndef OPENtls_NO_DEPRECATED_3_0
#  define Otls_STORE_F_FILE_CTRL                           0
#  define Otls_STORE_F_FILE_FIND                           0
#  define Otls_STORE_F_FILE_GET_PASS                       0
#  define Otls_STORE_F_FILE_LOAD                           0
#  define Otls_STORE_F_FILE_LOAD_TRY_DECODE                0
#  define Otls_STORE_F_FILE_NAME_TO_URI                    0
#  define Otls_STORE_F_FILE_OPEN                           0
#  define Otls_STORE_F_Otls_STORE_ATTACH_PEM_BIO           0
#  define Otls_STORE_F_Otls_STORE_EXPECT                   0
#  define Otls_STORE_F_Otls_STORE_FILE_ATTACH_PEM_BIO_INT  0
#  define Otls_STORE_F_Otls_STORE_FIND                     0
#  define Otls_STORE_F_Otls_STORE_GET0_LOADER_INT          0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_CERT           0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_CRL            0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_NAME           0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_NAME_DESCRIPTION 0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_PARAMS         0
#  define Otls_STORE_F_Otls_STORE_INFO_GET1_PKEY           0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_CERT            0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_CRL             0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_EMBEDDED        0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_NAME            0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_PARAMS          0
#  define Otls_STORE_F_Otls_STORE_INFO_NEW_PKEY            0
#  define Otls_STORE_F_Otls_STORE_INFO_SET0_NAME_DESCRIPTION 0
#  define Otls_STORE_F_Otls_STORE_INIT_ONCE                0
#  define Otls_STORE_F_Otls_STORE_LOADER_NEW               0
#  define Otls_STORE_F_Otls_STORE_OPEN                     0
#  define Otls_STORE_F_Otls_STORE_OPEN_INT                 0
#  define Otls_STORE_F_Otls_STORE_REGISTER_LOADER_INT      0
#  define Otls_STORE_F_Otls_STORE_SEARCH_BY_ALIAS          0
#  define Otls_STORE_F_Otls_STORE_SEARCH_BY_ISSUER_SERIAL  0
#  define Otls_STORE_F_Otls_STORE_SEARCH_BY_KEY_FINGERPRINT 0
#  define Otls_STORE_F_Otls_STORE_SEARCH_BY_NAME           0
#  define Otls_STORE_F_Otls_STORE_UNREGISTER_LOADER_INT    0
#  define Otls_STORE_F_TRY_DECODE_PARAMS                   0
#  define Otls_STORE_F_TRY_DECODE_PKCS12                   0
#  define Otls_STORE_F_TRY_DECODE_PKCS8ENCRYPTED           0
# endif

/*
 * Otls_STORE reason codes.
 */
# define Otls_STORE_R_AMBIGUOUS_CONTENT_TYPE              107
# define Otls_STORE_R_BAD_PASSWORD_READ                   115
# define Otls_STORE_R_ERROR_VERIFYING_PKCS12_MAC          113
# define Otls_STORE_R_FINGERPRINT_SIZE_DOES_NOT_MATCH_DIGEST 121
# define Otls_STORE_R_INVALID_SCHEME                      106
# define Otls_STORE_R_IS_NOT_A                            112
# define Otls_STORE_R_LOADER_INCOMPLETE                   116
# define Otls_STORE_R_LOADING_STARTED                     117
# define Otls_STORE_R_NOT_A_CERTIFICATE                   100
# define Otls_STORE_R_NOT_A_CRL                           101
# define Otls_STORE_R_NOT_A_KEY                           102
# define Otls_STORE_R_NOT_A_NAME                          103
# define Otls_STORE_R_NOT_PARAMETERS                      104
# define Otls_STORE_R_PASSPHRASE_CALLBACK_ERROR           114
# define Otls_STORE_R_PATH_MUST_BE_ABSOLUTE               108
# define Otls_STORE_R_SEARCH_ONLY_SUPPORTED_FOR_DIRECTORIES 119
# define Otls_STORE_R_UI_PROCESS_INTERRUPTED_OR_CANCELLED 109
# define Otls_STORE_R_UNREGISTERED_SCHEME                 105
# define Otls_STORE_R_UNSUPPORTED_CONTENT_TYPE            110
# define Otls_STORE_R_UNSUPPORTED_OPERATION               118
# define Otls_STORE_R_UNSUPPORTED_SEARCH_TYPE             120
# define Otls_STORE_R_URI_AUTHORITY_UNSUPPORTED           111

#endif
