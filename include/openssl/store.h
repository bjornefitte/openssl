/*
 * Copyright 2001-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_STORE_H
# define HEADER_STORE_H

# include <openssl/ossl_typ.h>
# include <openssl/pem.h>

# ifdef  __cplusplus
extern "C" {
# endif

/******************************************************************************
 *
 *  The main STORE functions.  It allows applications to open a channel
 *  to a resource with supported data (keys, certs, crls, ...), read the
 *  data a piece at a time and decide what to do with it, and finally close.
 *
 *****/

typedef struct store_ctx_st STORE_CTX;

/*
 * Open a channel given a URI.  The given password callback will be used any
 * time a password is needed, and will be passed the same callback data every
 * time it's needed in this context.
 *
 * Returns a context reference which represents the channel to communicate
 * through.
 */
STORE_CTX *STORE_open(const char *uri, pem_password_cb *pw_callback,
                      void *pw_callback_data);

/*
 * Read one data item (a key, a cert, a CRL) that is supported by the STORE
 * functionality, given a context.
 * Returns a STORE_INFO pointer, from which OpenSSL typed data can be extracted
 * with STORE_INFO_get0_PKEY(), STORE_INFO_get0_CERT(), ...
 * NULL is returned on error, which may include that the data found at the URI
 * can't be figured out for certain or is ambiguous.
 */
STORE_INFO *STORE_load(STORE_CTX *ctx);

/*
 * Check if end of data (end of file) is reached
 * Returns 1 on end, 0 otherwise.
 */
int STORE_eof(STORE_CTX *ctx);

/*
 * Close the channel
 * Returns 1 on success, 0 on error.
 */
int STORE_close(STORE_CTX *ctx);


/******************************************************************************
 *
 *  Function to register a loader for the given URI scheme.
 *  The loader receives all the main components of an URI except for the
 *  scheme.
 *
 *****/

typedef struct store_loader_st STORE_LOADER;
STORE_LOADER *STORE_LOADER_new(void);
int STORE_LOADER_set0_scheme(STORE_LOADER *store_loader, const char *scheme);
/* struct store_loader_st is defined differently by each loader */
typedef struct store_loader_ctx_st STORE_LOADER_CTX;
typedef STORE_LOADER_CTX *(*STORE_open_fn)(const char *authority,
                                           const char *path,
                                           const char *query,
                                           const char *fragment);
int STORE_LOADER_set_open(STORE_LOADER *store_loader,
                          STORE_open_fn store_open_function);
typedef STORE_INFO *(*STORE_load_fn)(STORE_LOADER_CTX *ctx,
                                     pem_password_cb *pw_callback,
                                     void *pw_callback_data);
int STORE_LOADER_set_load(STORE_LOADER *store_loader,
                          STORE_load_fn store_load_function);
typedef int (*STORE_eof_fn)(STORE_LOADER_CTX *ctx);
int STORE_LOADER_set_eof(STORE_LOADER *store_loader,
                           STORE_eof_fn store_eof_function);
typedef int (*STORE_close_fn)(STORE_LOADER_CTX *ctx);
int STORE_LOADER_set_close(STORE_LOADER *store_loader,
                           STORE_close_fn store_close_function);
int STORE_LOADER_free(STORE_LOADER *store_loader);

int STORE_register_loader(STORE_LOADER *loader);
STORE_LOADER *STORE_unregister_loader(const char *scheme);

/******************************************************************************
 *
 *  Extracting OpenSSL types from STORE_INFOs and creating new STORE_INFOs
 *
 *****/

/*
 * Types of data that can be stored in a STORE_INFO.
 * STORE_INFO_NAME is typically found when getting a listing of
 * available "files" / "tokens" / what have you.
 */
enum STORE_INFO_types {
    STORE_INFO_NAME = 0,         /* char * */
    STORE_INFO_DSAPARAMS,        /* DSA * */
    STORE_INFO_ECPARAMS,         /* EC_GROUP * */
    STORE_INFO_PKEY,             /* EVP_PKEY * */
    STORE_INFO_CERT,             /* X509 * */
    STORE_INFO_CRL               /* X509_CRL * */
};

/*
 * Functions to generate STORE_INFOs, one function for each type we
 * support having in them.  Along with each of them, one macro that
 * can be used to determine what types are supported.
 *
 * In all cases, ownership of the object is transfered to the STORE_INFO
 * and will therefore be freed when the STORE_INFO is freed.
 */
STORE_INFO *STORE_INFO_new_NAME(char *name);
STORE_INFO *STORE_INFO_new_DSAPARAMS(DSA *dsa_params);
STORE_INFO *STORE_INFO_new_ECPARAMS(EC_GROUP *ec_group);
STORE_INFO *STORE_INFO_new_PKEY(EVP_PKEY *pkey);
STORE_INFO *STORE_INFO_new_CERT(X509 *x509);
STORE_INFO *STORE_INFO_new_CRL(X509_CRL *crl);

/*
 * Functions to try to extract data from a STORE_INFO.
 */
int STORE_INFO_get_type(const STORE_INFO *store_info);
const char *STORE_INFO_get0_NAME(const STORE_INFO *store_info);
const DSA *STORE_INFO_get0_DSAPARAMS(const STORE_INFO *store_info);
const EC_GROUP *STORE_INFO_get0_ECPARAMS(const STORE_INFO *store_info);
const EVP_PKEY *STORE_INFO_get0_PKEY(const STORE_INFO *store_info);
const X509 *STORE_INFO_get0_CERT(const STORE_INFO *store_info);
const X509_CRL *STORE_INFO_get0_CRL(const STORE_INFO *store_info);

const char *STORE_INFO_type_string(int type);

/*
 * Free the STORE_INFO
 */
void STORE_INFO_free(STORE_INFO *store_info);


/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_STORE_strings(void);

/* Error codes for the STORE functions. */

/* Function codes. */
# define STORE_F_FILE_LOAD                                108
# define STORE_F_FILE_OPEN                                109
# define STORE_F_STORE_FILE_HANDLER_NEW                   112
# define STORE_F_STORE_FILE_HANDLER_SET0_NAME             113
# define STORE_F_STORE_FILE_HANDLER_SET_DESTROY_CTX       118
# define STORE_F_STORE_FILE_HANDLER_SET_EOF               119
# define STORE_F_STORE_FILE_HANDLER_SET_TRY_DECODE        114
# define STORE_F_STORE_FILE_UNREGISTER_HANDLER_INT        110
# define STORE_F_STORE_INFO_NEW_CERT                      100
# define STORE_F_STORE_INFO_NEW_CRL                       101
# define STORE_F_STORE_INFO_NEW_DSAPARAMS                 116
# define STORE_F_STORE_INFO_NEW_ECPARAMS                  117
# define STORE_F_STORE_INFO_NEW_NAME                      102
# define STORE_F_STORE_INFO_NEW_PKEY                      103
# define STORE_F_STORE_INIT_ONCE                          104
# define STORE_F_STORE_LOADER_NEW                         105
# define STORE_F_STORE_OPEN                               106
# define STORE_F_STORE_UNREGISTER_LOADER_INT              107
# define STORE_F_TRY_DECODE_PKCS8PRIVATEKEY               111

/* Reason codes. */
# define STORE_R_AMBIGUOUS_CONTENT_TYPE                   101
# define STORE_R_BAD_PASSWORD_READ                        102
# define STORE_R_UNREGISTERED_NAME                        103
# define STORE_R_UNREGISTERED_SCHEME                      100
# define STORE_R_UNSUPPORTED_CONTENT_TYPE                 104
# define STORE_R_URI_AUTHORITY_UNSUPPORED                 105
# define STORE_R_URI_FRAGMENT_UNSUPPORED                  106
# define STORE_R_URI_QUERY_UNSUPPORED                     107

# ifdef  __cplusplus
}
# endif
#endif
