/*
 * Copyright 2001-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>

#include <openssl/bio.h>
#include <openssl/dsa.h>         /* For d2i_DSAPrivateKey */
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>      /* For the PKCS8 stuff o.O */
#include <openssl/rsa.h>         /* For d2i_RSAPrivateKey */
#include <openssl/safestack.h>
#include <openssl/store.h>
#include <openssl/ui.h>
#include <openssl/x509.h>        /* For the PKCS8 stuff o.O */
#include "internal/asn1_int.h"
#include "store_local.h"

#include "e_os.h"

/******************************************************************************
 *
 *  Password prompting
 *
 *****/

static char *file_get_pass(const UI_METHOD *ui_method, char *pass,
                           size_t maxsize, const char *prompt_info, void *data)
{
    UI *ui = UI_new();
    char *prompt = NULL;

    if (ui == NULL) {
        STOREerr(STORE_F_FILE_GET_PASS, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if (ui_method)
        UI_set_method(ui, ui_method);
    UI_add_user_data(ui, data);

    if ((prompt = UI_construct_prompt(ui, "pass phrase", prompt_info)) == NULL) {
        STOREerr(STORE_F_FILE_GET_PASS, ERR_R_MALLOC_FAILURE);
        pass = NULL;
    } else if (!UI_add_input_string(ui, prompt, UI_INPUT_FLAG_DEFAULT_PWD,
                                    pass, 0, maxsize - 1)) {
        STOREerr(STORE_F_FILE_GET_PASS, ERR_R_UI_LIB);
        pass = NULL;
    } else {
        switch (UI_process(ui)) {
        case -2:
            STOREerr(STORE_F_FILE_GET_PASS,
                     STORE_R_UI_PROCESS_INTERRUPTED_OR_CANCELLED);
            pass = NULL;
            break;
        case -1:
            STOREerr(STORE_F_FILE_GET_PASS, ERR_R_UI_LIB);
            pass = NULL;
            break;
        default:
            break;
        }
    }

    OPENSSL_free(prompt);
    UI_free(ui);
    return pass;
}

struct pem_pass_data {
    const UI_METHOD *ui_method;
    void *data;
    const char *prompt_info;
};
static int file_fill_pem_pass_data(struct pem_pass_data *pass_data,
                                   const char *prompt_info,
                                   const UI_METHOD *ui_method, void *ui_data)
{
    if (pass_data == NULL)
        return 0;
    pass_data->ui_method = ui_method;
    pass_data->data = ui_data;
    pass_data->prompt_info = prompt_info;
    return 1;
}
static int file_get_pem_pass(char *buf, int num, int w, void *data)
{
    struct pem_pass_data *pass_data = data;
    char *pass = file_get_pass(pass_data->ui_method, buf, num,
                               pass_data->prompt_info, pass_data->data);

    return pass == NULL ? 0 : strlen(pass);
}

/******************************************************************************
 *
 *  The file scheme handlers
 *
 *****/

/*
 * The try_decode function is called to check if the blob of data can
 * be used by this handler, and if it can, decodes it into a supported
 * OpenSSL and returns a STORE_INFO with the recorded data.
 * Input:
 *    pem_name:     If this blob comes from a PEM file, this holds
 *                  the PEM name.  If it comes from another type of
 *                  file, this is NULL.
 *    pem_header:   If this blob comes from a PEM file, this holds
 *                  the PEM headers.  If it comes from another type of
 *                  file, this is NULL.
 *    blob:         The blob of data to match with what this handler
 *                  can use.
 *    len:          The length of the blob.
 *    handler_ctx:  For a handler marked repeatable, this pointer can
 *                  be used to create a context for the handler.  IT IS
 *                  THE HANDLER'S RESPONSIBILITY TO CREATE AND DESTROY
 *                  THIS CONTEXT APPROPRIATELY, i.e. create on first call
 *                  and destroy when about to return NULL.
 *    ui_method:    Application UI method for getting a password, pin
 *                  or any other interactive data.
 *    ui_data:      Application data to be passed to ui_method when
 *                  it's called.
 * Output:
 *    a STORE_INFO
 */
typedef STORE_INFO *(*STORE_FILE_try_decode_fn)(const char *pem_name,
                                                const char *pem_header,
                                                const unsigned char *blob,
                                                size_t len, void **handler_ctx,
                                                const UI_METHOD *ui_method,
                                                void *ui_data);
/*
 * The eof function should return 1 if there's no more data to be found
 * with the handler_ctx, otherwise 0.  This is only used when the handler is
 * marked repeatable.
 */
typedef int (*STORE_FILE_eof_fn)(void *handler_ctx);
/*
 * The destroy_ctx function is used to destroy the handler_ctx that was
 * intiated by a repeatable try_decode fuction.  This is only used when
 * the handler is marked repeatable.
 */
typedef void (*STORE_FILE_destroy_ctx_fn)(void **handler_ctx);

typedef struct store_file_handler_st {
    const char *name;
    STORE_FILE_try_decode_fn try_decode;
    STORE_FILE_eof_fn eof;
    STORE_FILE_destroy_ctx_fn destroy_ctx;

    /* flags */
    int repeatable;
} STORE_FILE_HANDLER;

int pem_check_suffix(const char *pem_str, const char *suffix);
static STORE_INFO *try_decode_PrivateKey(const char *pem_name,
                                         const char *pem_header,
                                         const unsigned char *blob,
                                         size_t len, void **pctx,
                                         const UI_METHOD *ui_method,
                                         void *ui_data)
{
    STORE_INFO *store_info = NULL;
    EVP_PKEY *pkey = NULL;
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;

    if (pem_name != NULL) {
        int slen;

        if ((slen = pem_check_suffix(pem_name, "PRIVATE KEY")) > 0
            && (ameth = EVP_PKEY_asn1_find_str(NULL, pem_name, slen)) != NULL)
            pkey = d2i_PrivateKey(ameth->pkey_id, NULL, &blob, len);
    } else {
        int i;

        for (i = 0; i < EVP_PKEY_asn1_get_count(); i++) {
            ameth = EVP_PKEY_asn1_get0(i);
            if (ameth->pkey_flags & ASN1_PKEY_ALIAS)
                continue;
            pkey = d2i_PrivateKey(ameth->pkey_id, NULL, &blob, len);
            if (pkey != NULL)
                break;
        }
    }
    if (pkey == NULL)
        /* No match */
        return NULL;

    store_info = STORE_INFO_new_PKEY(pkey);

    return store_info;
}
static STORE_FILE_HANDLER PrivateKey_handler = {
    "PrivateKey",
    try_decode_PrivateKey
};

static STORE_INFO *try_decode_PUBKEY(const char *pem_name,
                                     const char *pem_header,
                                     const unsigned char *blob, size_t len,
                                     void **pctx, const UI_METHOD *ui_method,
                                     void *ui_data)
{
    STORE_INFO *store_info = NULL;
    EVP_PKEY *pkey = NULL;


    if (pem_name != NULL && strcmp(pem_name, PEM_STRING_PUBLIC) != 0)
        /* No match */
        return NULL;

    if ((pkey = d2i_PUBKEY(NULL, &blob, len)) != NULL)
        store_info = STORE_INFO_new_PKEY(pkey);

    return store_info;
}
static STORE_FILE_HANDLER PUBKEY_handler = {
    "PUBKEY",
    try_decode_PUBKEY
};

static STORE_INFO *try_decode_params(const char *pem_name,
                                     const char *pem_header,
                                     const unsigned char *blob, size_t len,
                                     void **pctx, const UI_METHOD *ui_method,
                                     void *ui_data)
{
    STORE_INFO *store_info = NULL;
    EVP_PKEY *pkey = EVP_PKEY_new();
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;
    int ok = 0;

    if (pkey == NULL) {
        STOREerr(STORE_F_TRY_DECODE_PARAMS, ERR_R_EVP_LIB);
        EVP_PKEY_free(pkey);
        return (NULL);
    }

    if (pem_name != NULL) {
        int slen;

        if ((slen = pem_check_suffix(pem_name, "PARAMETERS")) > 0
            && EVP_PKEY_set_type_str(pkey, pem_name, slen)
            && (ameth = EVP_PKEY_get0_asn1(pkey)) != NULL
            && ameth->param_decode != NULL
            && ameth->param_decode(pkey, &blob, len)) {
            ok = 1;
        }
    } else {
        int i;

        for (i = 0; i < EVP_PKEY_asn1_get_count(); i++) {
            ameth = EVP_PKEY_asn1_get0(i);
            if (ameth->pkey_flags & ASN1_PKEY_ALIAS)
                continue;
            if (EVP_PKEY_set_type(pkey, ameth->pkey_id)
                && (ameth = EVP_PKEY_get0_asn1(pkey)) != NULL
                && ameth->param_decode != NULL
                && ameth->param_decode(pkey, &blob, len)) {
                ok = 1;
                break;
            }
        }
    }

    if (ok)
        store_info = STORE_INFO_new_PARAMS(pkey);
    else
        EVP_PKEY_free(pkey);

    return store_info;
}
static STORE_FILE_HANDLER params_handler = {
    "params",
    try_decode_params
};

static STORE_INFO *try_decode_X509Certificate(const char *pem_name,
                                              const char *pem_header,
                                              const unsigned char *blob,
                                              size_t len, void **pctx,
                                              const UI_METHOD *ui_method,
                                              void *ui_data)
{
    STORE_INFO *store_info = NULL;
    X509 *cert = NULL;


    if (pem_name != NULL
        && strcmp(pem_name, PEM_STRING_X509_OLD) != 0
        && strcmp(pem_name, PEM_STRING_X509) != 0
        && strcmp(pem_name, PEM_STRING_X509_TRUSTED) != 0)
        /* No match */
        return NULL;

    if ((cert = d2i_X509(NULL, &blob, len)) != NULL)
        store_info = STORE_INFO_new_CERT(cert);

    return store_info;
}
static STORE_FILE_HANDLER X509Certificate_handler = {
    "X509Certificate",
    try_decode_X509Certificate
};

static STORE_INFO *try_decode_X509CRL(const char *pem_name,
                                      const char *pem_header,
                                      const unsigned char *blob,
                                      size_t len, void **pctx,
                                      const UI_METHOD *ui_method, void *ui_data)
{
    STORE_INFO *store_info = NULL;
    X509_CRL *crl = NULL;


    if (pem_name != NULL
        && strcmp(pem_name, PEM_STRING_X509_CRL) != 0)
        /* No match */
        return NULL;

    if ((crl = d2i_X509_CRL(NULL, &blob, len)) != NULL)
        store_info = STORE_INFO_new_CRL(crl);

    return store_info;
}
static STORE_FILE_HANDLER X509CRL_handler = {
    "X509CRL",
    try_decode_X509CRL
};

static const STORE_FILE_HANDLER *file_handlers[] = {
    &X509Certificate_handler,
    &X509CRL_handler,
    &params_handler,
    &PUBKEY_handler,
    &PrivateKey_handler,
};


/******************************************************************************
 *
 *  The loader itself
 *
 *****/

struct store_loader_ctx_st {
    BIO *file;
    int is_pem;

    /* The following are used when the handler is marked as repeatable */
    const STORE_FILE_HANDLER *last_handler;
    void *last_handler_ctx;
};

static STORE_LOADER_CTX *file_open(const char *scheme, const char *user,
                                   const char *password, const char *host,
                                   const char *service, const char *path,
                                   const char *query, const char *fragment)
{
    BIO *buff = NULL;
    char peekbuf[4096];
    STORE_LOADER_CTX *ctx = NULL;

    if (user != NULL || password != NULL
        || (host != NULL && *host != '\0' && strcmp(host, "localhost") != 0)
        || service != NULL) {
        STOREerr(STORE_F_FILE_OPEN, STORE_R_URI_AUTHORITY_UNSUPPORED);
        return NULL;
    }
    /*
     * Future development may allow a query to select the appropriate PEM
     * object in case a PEM file is loaded.
     */
    if (query != NULL) {
        STOREerr(STORE_F_FILE_OPEN, STORE_R_URI_QUERY_UNSUPPORED);
        return NULL;
    }
    /*
     * Future development may allow a numeric fragment to select which
     * object to return in case a PEM file is loaded.
     */
    if (fragment != NULL) {
        STOREerr(STORE_F_FILE_OPEN, STORE_R_URI_FRAGMENT_UNSUPPORED);
        return NULL;
    }

    /*
     * If the scheme "file" was an explicit part of the URI, the path must
     * be absolute.  So says RFC 8089
     */
    if (scheme != NULL           /* We only know the scheme "file" */
        && path[0] != '/') {
        STOREerr(STORE_F_FILE_OPEN, STORE_R_PATH_MUST_BE_ABSOLUTE);
        return NULL;
    }

#ifdef _WIN32
    if (scheme != NULL) {        /* We only know the scheme "file" */
        if (path[0] == '/' && path[2] == ':' && path[3] == '/')
            path++;
    }
#endif

    ctx = OPENSSL_zalloc(sizeof(*ctx));
    if (ctx == NULL) {
        STOREerr(STORE_F_FILE_OPEN, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    if ((buff = BIO_new(BIO_f_buffer())) == NULL)
        goto err;
    if ((ctx->file = BIO_new_file(path, "rb")) == NULL) {
        goto err;
    }
    ctx->file = BIO_push(buff, ctx->file);
    if (BIO_buffer_peek(ctx->file, peekbuf, sizeof(peekbuf)-1) > 0) {
        peekbuf[sizeof(peekbuf)-1] = '\0';
        if (strstr(peekbuf, "-----BEGIN ") != NULL)
            ctx->is_pem = 1;
    }

    return ctx;
 err:
    if (buff != NULL)
        BIO_free(buff);
    OPENSSL_free(ctx);
    return NULL;
}

static int file_eof(STORE_LOADER_CTX *ctx);
static STORE_INFO *file_load(STORE_LOADER_CTX *ctx, const UI_METHOD *ui_method,
                             void *ui_data)
{
    STORE_INFO *result = NULL;
    int matchcount = -1;

    if (ctx->last_handler != NULL) {
        result = ctx->last_handler->try_decode(NULL, NULL, NULL, 0,
                                               &ctx->last_handler_ctx,
                                               ui_method, ui_data);

        if (result != NULL)
            return result;

        ctx->last_handler->destroy_ctx(&ctx->last_handler_ctx);
        ctx->last_handler_ctx = NULL;
        ctx->last_handler = NULL;
    }

    do {
        char *pem_name = NULL;      /* PEM record name */
        char *pem_header = NULL;    /* PEM record header */
        unsigned char *data = NULL; /* DER encoded data */
        BUF_MEM *mem = NULL;
        long len = 0;               /* DER encoded data length */
        int r = 0;

        matchcount = -1;
        if (ctx->is_pem) {
            r = PEM_read_bio(ctx->file, &pem_name, &pem_header, &data, &len);
            if (r <= 0)
                goto end;

            if (strlen(pem_header) > 10) {
                EVP_CIPHER_INFO cipher;
                struct pem_pass_data pass_data;

                if (!PEM_get_EVP_CIPHER_INFO(pem_header, &cipher)
                    || !file_fill_pem_pass_data(&pass_data, "PEM", ui_method,
                                                ui_data)
                    || !PEM_do_header(&cipher, data, &len, file_get_pem_pass,
                                      &pass_data)) {
                    goto err;
                }
            }
        } else {
            if ((len = asn1_d2i_read_bio(ctx->file, &mem)) < 0)
                goto err;

            data = (unsigned char *)mem->data;
            len = (long)mem->length;
        }

        result = NULL;

        {
            size_t i = 0;
            void *handler_ctx = NULL;
            const STORE_FILE_HANDLER **matching_handlers =
                OPENSSL_zalloc(sizeof(*matching_handlers)
                               * OSSL_NELEM(file_handlers));

            if (matching_handlers == NULL) {
                STOREerr(STORE_F_FILE_LOAD, ERR_R_MALLOC_FAILURE);
                goto err;
            }

            matchcount = 0;
            for (i = 0; i < OSSL_NELEM(file_handlers); i++) {
                const STORE_FILE_HANDLER *handler = file_handlers[i];
                void *tmp_handler_ctx = NULL;
                STORE_INFO *tmp_result = handler->try_decode(pem_name,
                                                             pem_header,
                                                             data, len,
                                                             &tmp_handler_ctx,
                                                             ui_method,
                                                             ui_data);

                if (tmp_result == NULL) {
                    STOREerr(STORE_F_FILE_LOAD, STORE_R_IS_NOT_A);
                    ERR_add_error_data(1, handler->name);
                } else {
                    if (matching_handlers)
                        matching_handlers[matchcount] = handler;

                    if (handler_ctx)
                        handler->destroy_ctx(&handler_ctx);
                    handler_ctx = tmp_handler_ctx;

                    if (++matchcount == 1) {
                        result = tmp_result;
                        tmp_result = NULL;
                    } else {
                        /* more than one match => ambiguous, kill any result */
                        STORE_INFO_free(result);
                        STORE_INFO_free(tmp_result);
                        if (handler->destroy_ctx != NULL)
                            handler->destroy_ctx(&handler_ctx);
                        handler_ctx = NULL;
                        result = NULL;
                    }
                }
            }

            if (matchcount > 1)
                STOREerr(STORE_F_FILE_LOAD, STORE_R_AMBIGUOUS_CONTENT_TYPE);
            if (matchcount == 0)
                STOREerr(STORE_F_FILE_LOAD, STORE_R_UNSUPPORTED_CONTENT_TYPE);
            else if (matching_handlers[0]->repeatable) {
                ctx->last_handler = matching_handlers[0];
                ctx->last_handler_ctx = handler_ctx;
                mem = NULL;
                data = NULL;
            }

            OPENSSL_free(matching_handlers);
        }

        if (result)
            ERR_clear_error();

     err:
        OPENSSL_free(pem_name);
        OPENSSL_free(pem_header);
        if (mem == NULL)
            OPENSSL_free(data);
        else
            BUF_MEM_free(mem);
    } while (matchcount == 0 && !file_eof(ctx));

    /* We bail out on ambiguity */
    if (matchcount > 1)
        return NULL;

 end:
    if (result == NULL)
        result = STORE_INFO_new_ENDOFDATA();
    return result;
}

static int file_eof(STORE_LOADER_CTX *ctx)
{
    if (ctx->last_handler != NULL
        && !ctx->last_handler->eof(ctx->last_handler_ctx))
        return 0;
    return BIO_eof(ctx->file);
}

static int file_close(STORE_LOADER_CTX *ctx)
{
    if (ctx->last_handler != NULL) {
        ctx->last_handler->destroy_ctx(&ctx->last_handler_ctx);
        ctx->last_handler_ctx = NULL;
        ctx->last_handler = NULL;
    }
    BIO_free_all(ctx->file);
    OPENSSL_free(ctx);
    return 1;
}

static STORE_LOADER store_file_loader =
    {
        "file",
        file_open,
        file_load,
        file_eof,
        file_close
    };

int store_file_loader_init(void)
{
    return store_register_loader_int(&store_file_loader);
}
