/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/store.h>

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

# define ERR_FUNC(func) ERR_PACK(ERR_LIB_STORE,func,0)
# define ERR_REASON(reason) ERR_PACK(ERR_LIB_STORE,0,reason)

static ERR_STRING_DATA STORE_str_functs[] = {
    {ERR_FUNC(STORE_F_FILE_GET_PASS), "file_get_pass"},
    {ERR_FUNC(STORE_F_FILE_LOAD), "file_load"},
    {ERR_FUNC(STORE_F_FILE_OPEN), "file_open"},
    {ERR_FUNC(STORE_F_STORE_GET0_LOADER_INT), "store_get0_loader_int"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_CERT), "STORE_INFO_get1_CERT"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_CRL), "STORE_INFO_get1_CRL"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_NAME), "STORE_INFO_get1_NAME"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_NAME_DESCRIPTION),
     "STORE_INFO_get1_NAME_description"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_PARAMS), "STORE_INFO_get1_PARAMS"},
    {ERR_FUNC(STORE_F_STORE_INFO_GET1_PKEY), "STORE_INFO_get1_PKEY"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_CERT), "STORE_INFO_new_CERT"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_CRL), "STORE_INFO_new_CRL"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_ENDOFDATA), "STORE_INFO_new_ENDOFDATA"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_NAME), "STORE_INFO_new_NAME"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_PARAMS), "STORE_INFO_new_PARAMS"},
    {ERR_FUNC(STORE_F_STORE_INFO_NEW_PKEY), "STORE_INFO_new_PKEY"},
    {ERR_FUNC(STORE_F_STORE_INFO_SET0_NAME_DESCRIPTION),
     "STORE_INFO_set0_NAME_description"},
    {ERR_FUNC(STORE_F_STORE_INIT_ONCE), "store_init_once"},
    {ERR_FUNC(STORE_F_STORE_LOADER_NEW), "STORE_LOADER_new"},
    {ERR_FUNC(STORE_F_STORE_OPEN), "STORE_open"},
    {ERR_FUNC(STORE_F_STORE_OPEN_INT), "STORE_OPEN_INT"},
    {ERR_FUNC(STORE_F_STORE_REGISTER_LOADER_INT),
     "store_register_loader_int"},
    {ERR_FUNC(STORE_F_STORE_UNREGISTER_LOADER_INT),
     "store_unregister_loader_int"},
    {ERR_FUNC(STORE_F_TRY_DECODE_PARAMS), "try_decode_params"},
    {0, NULL}
};

static ERR_STRING_DATA STORE_str_reasons[] = {
    {ERR_REASON(STORE_R_AMBIGUOUS_CONTENT_TYPE), "ambiguous content type"},
    {ERR_REASON(STORE_R_INVALID_SCHEME), "invalid scheme"},
    {ERR_REASON(STORE_R_IS_NOT_A), "is not a"},
    {ERR_REASON(STORE_R_NOT_A_CERTIFICATE), "not a certificate"},
    {ERR_REASON(STORE_R_NOT_A_CRL), "not a crl"},
    {ERR_REASON(STORE_R_NOT_A_KEY), "not a key"},
    {ERR_REASON(STORE_R_NOT_A_NAME), "not a name"},
    {ERR_REASON(STORE_R_NOT_PARAMETERS), "not parameters"},
    {ERR_REASON(STORE_R_PATH_MUST_BE_ABSOLUTE), "path must be absolute"},
    {ERR_REASON(STORE_R_UI_PROCESS_INTERRUPTED_OR_CANCELLED),
     "ui process interrupted or cancelled"},
    {ERR_REASON(STORE_R_UNREGISTERED_SCHEME), "unregistered scheme"},
    {ERR_REASON(STORE_R_UNSUPPORTED_CONTENT_TYPE),
     "unsupported content type"},
    {ERR_REASON(STORE_R_URI_AUTHORITY_UNSUPPORED),
     "uri authority unsuppored"},
    {0, NULL}
};

#endif

int ERR_load_STORE_strings(void)
{
#ifndef OPENSSL_NO_ERR

    if (ERR_func_error_string(STORE_str_functs[0].error) == NULL) {
        ERR_load_strings(0, STORE_str_functs);
        ERR_load_strings(0, STORE_str_reasons);
    }
#endif
    return 1;
}
