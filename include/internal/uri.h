/*
 * Copyright 2001-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_URI_H
# define HEADER_URI_H

# ifdef  __cplusplus
extern "C" {
# endif

/*
 * Decode the URI given in |uri| into its 5 major parts, based on RFC 3986.
 * |scheme|, |authority|, |path|, |query|, |fragment| are pointers to C
 * strings and for each of those being non-NULL, the string will be allocated
 * and filled with the corresponding part of the URI, if that part is present.
 *
 * returns 0 on failure, 1 on success
 */
int OPENSSL_decode_uri(const char *uri, char **scheme, char **authority,
                       char **path, char **query, char **fragment);

/*
 * Decode the authority part of a URI, given in |authority| into its 4 parts,
 * based on RFC 3986.  |user|, |password|, |host|, |service| are pointers to C
 * strings and for each of those being non-NULL, the string will be allocated
 * and filled with the corresponding part of the authority, if that part is
 * present.
 *
 * returns 0 on failure, 1 on success
 */
int OPENSSL_decode_authority(const char *authority, char **user,
                             char **password, char **host, char **service);

/*
 * Given a C string |str| that contains a part of a URI, convert all %xx to
 * bytes with the value of the hex code xx.  This happens inline.  If there
 * is a failure, the string will potentially be partially decoded and should
 * not be used.
 *
 * returns 0 on failure, 1 on success
 */
int OPENSSL_percent_decode_inline(char *str);

/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_URI_strings(void);

/* Error codes for the URI functions. */

/* Function codes. */
# define URI_F_EXTRACT_AUTHORITY                          100
# define URI_F_EXTRACT_FRAGMENT                           101
# define URI_F_EXTRACT_HOSTINFO                           111
# define URI_F_EXTRACT_PATH_ABEMPTY                       102
# define URI_F_EXTRACT_PATH_ABSOLUTE                      103
# define URI_F_EXTRACT_PATH_EMPTY                         104
# define URI_F_EXTRACT_PATH_NOSCHEME                      105
# define URI_F_EXTRACT_PATH_ROOTLESS                      106
# define URI_F_EXTRACT_QUERY                              107
# define URI_F_EXTRACT_SCHEME                             108
# define URI_F_EXTRACT_USERINFO                           112
# define URI_F_OPENSSL_DECODE_AUTHORITY                   113
# define URI_F_OPENSSL_DECODE_URI                         109
# define URI_F_OPENSSL_PERCENT_DECODE_INLINE              110

/* Reason codes. */
# define URI_R_FAILED_TO_DECODE_URI                       100
# define URI_R_INVALID_CHARACTER_IN_URI                   101
# define URI_R_INVALID_PERCENT_CODE                       104
# define URI_R_MALFORMED_HOST_IN_URI                      102
# define URI_R_MALFORMED_PATH_IN_URI                      103
# define URI_R_PERCENT_NUL_UNSUPPORTED                    105

# ifdef  __cplusplus
}
# endif
#endif
