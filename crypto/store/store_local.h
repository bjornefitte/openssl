/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "internal/thread_once.h"
#include <openssl/dsa.h>
#include <openssl/engine.h>
#include <openssl/evp.h>
#include <openssl/lhash.h>
#include <openssl/x509.h>
#include <openssl/store.h>

/******************************************************************************
 *
 * STORE_INFO stuff
 *
 *****/

struct store_info_st {
    int type;
    union {
        void *data;              /* used internally as generic pointer */

        struct {
            BUF_MEM *blob;
            char *pem_name;
        } decoded;               /* when type == STORE_INFO_DECODED */

        char *name;              /* when type == STORE_INFO_NAME */
        EVP_PKEY *params;        /* when type == STORE_INFO_PARAMS */
        EVP_PKEY *pkey;          /* when type == STORE_INFO_PKEY */
        X509 *x509;              /* when type == STORE_INFO_X509 */
        X509_CRL *crl;           /* when type == STORE_INFO_CRL */
    } _;
};

DEFINE_STACK_OF(STORE_INFO)

/*
 * DECODED is a special type of STORE_INFO, specially for the file handlers.
 * It should never reach a calling application or any engine.  However, it
 * can be used by a STORE_FILE_HANDLER's try_decode function to signal that
 * it has decoded the incoming blob into a new blob, and that the attempted
 * decoding should be immediately restarted with the new blob, using the new
 * PEM name.
 */
/*
 * Because this is an internal type, we don't make it part of the enum that
 * applications and engines will see.
 */
#define STORE_INFO_DECODED     -1
STORE_INFO *store_info_new_DECODED(const char *new_pem_name, BUF_MEM *decoded);
BUF_MEM *store_info_get0_DECODED_buffer(STORE_INFO *store_info);
char *store_info_get0_DECODED_pem_name(STORE_INFO *store_info);

/******************************************************************************
 *
 * STORE_SEARCH stuff
 *
 *****/

struct store_search_st {
    enum STORE_SEARCH_types type;

    /* Use by STORE_SEARCH_BY_NAME and STORE_SEARCH_BY_ISSUER_SERIAL */
    X509_NAME *name;

    /* Use by STORE_SEARCH_BY_ISSUER_SERIAL */
    const ASN1_INTEGER *serial;

    /* Use by STORE_SEARCH_BY_KEY_FINGERPRINT and STORE_SEARCH_BY_ALIAS */
    const unsigned char *string;
    size_t stringlength;
};

/******************************************************************************
 *
 * STORE_LOADER stuff
 *
 *****/

int store_register_loader_int(STORE_LOADER *loader);
STORE_LOADER *store_unregister_loader_int(const char *scheme);

/* loader stuff */
struct store_loader_st {
    const char *scheme;
    ENGINE *engine;
    STORE_open_fn open;
    STORE_expect_fn expect;
    STORE_find_fn find;
    STORE_load_fn load;
    STORE_eof_fn eof;
    STORE_close_fn close;
};
DEFINE_LHASH_OF(STORE_LOADER);

const STORE_LOADER *store_get0_loader_int(const char *scheme);
void destroy_loaders_int(void);

/******************************************************************************
 *
 * STORE init stuff
 *
 *****/

int store_init_once(void);
int store_file_loader_init(void);
