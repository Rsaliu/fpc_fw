#ifndef __LOGIN_HANDLER_H__
#define __LOGIN_HANDLER_H__
#include "webserver.h"
#include "esp_http_server.h"
#include <common_headers.h>

esp_err_t login_handler(httpd_req_t *req);

#endif // __LOGIN_HANDLER_H__