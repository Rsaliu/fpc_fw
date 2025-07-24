#ifndef __SESSION_H__
#define __SESSION_H__
#include <common_headers.h>
#include "esp_http_server.h"
#define MAX_SESSIONS 8
#define USER_MAX     32
#define TOKEN_LEN    64
#define SESSION_LIFETIME_US (60LL * 60 * 1000000) // 1 hour

typedef struct {
    bool     used;
    char     username[USER_MAX];
    char     token[TOKEN_LEN + 1];
    int64_t  expires_us;
} session_t;

session_t* create_session(const char *user);
void set_session_cookie(httpd_req_t *req, const session_t *s);
session_t* find_session_by_token(const char *tok);
#endif // __SESSION_H__