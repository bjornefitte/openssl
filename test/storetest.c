/*
 * Copyright 2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/store_cache.h>
#include "testutil.h"

static size_t files_n;
static char **files;

static int test_store_cache()
{
    int ret = 1;
    size_t i;
    OSSL_STORE_CACHE *cache;

    if ((cache = OSSL_STORE_CACHE_new()) == NULL)
        return 0;

    for (i = 0; i < files_n; i++) {
        size_t items = 0, items2 = 0;
        OSSL_STORE_CTX *ctx = NULL;


        /* Fetch contents from the actual files */
        ctx = OSSL_STORE_CACHED_open(cache, files[i], 0, NULL, NULL, NULL,
                                     NULL);

        if (ctx == NULL)
            continue;

        for (;;) {
            OSSL_STORE_INFO *info = OSSL_STORE_load(ctx);

            if (info == NULL)
                break;

            items++;
            OSSL_STORE_INFO_free(info);
        }
        OSSL_STORE_close(ctx);

        TEST_info("%s: %zu items found", files[i], items);

        ERR_clear_error();

        /*
         * This time, only pilfer the cache and check that the amount of
         * objects is that same as above
         */
        ctx = OSSL_STORE_CACHED_open(cache, files[i], 1,
                                     NULL, NULL, NULL, NULL);

        if (ctx == NULL)
            continue;

        for (;;) {
            OSSL_STORE_INFO *info = OSSL_STORE_load(ctx);

            if (info == NULL)
                break;

            items2++;
            OSSL_STORE_INFO_free(info);
        }
        OSSL_STORE_close(ctx);

        TEST_info("%s: %zu items found in cache", files[i], items);

        ERR_clear_error();

        if (!TEST_size_t_eq(items, items2)) {
            TEST_error("Different amount of cached elements "
                       "than of originally loaded elements");
            ret = 0;
        }
    }

    OSSL_STORE_CACHE_free(cache);
    return ret;
}

int test_main(int argc, char *argv[])
{
    files_n = argc - 1;
    files = &argv[1];

    ADD_TEST(test_store_cache);

    return run_tests(argv[0]);
}
