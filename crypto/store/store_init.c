/*
 * Copyright 2001-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/err.h>
#include "store_local.h"

static CRYPTO_ONCE store_init = CRYPTO_ONCE_STATIC_INIT;
DEFINE_RUN_ONCE_STATIC(do_store_init)
{
    return OPENSSL_init_crypto(0, NULL)
        && OPENSSL_atexit(destroy_loaders_int);
}

int store_init_once()
{
    if (!RUN_ONCE(&store_init, do_store_init)) {
        STOREerr(STORE_F_STORE_INIT_ONCE, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    return 1;
}

