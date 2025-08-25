#ifndef __LOGOUT_HANDLER_H__
#define __LOGOUT_HANDLER_H__
#include "esp_err.h"
#include "esp_http_server.h"
esp_err_t logout_handler(httpd_req_t *req);

#endif // __LOGOUT_HANDLER_H__