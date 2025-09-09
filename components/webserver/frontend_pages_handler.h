#ifndef __FRONT_END_HANDLER_H__
#define __FRONT_END_HANDLER_H__
#include "esp_http_server.h"
#include "esp_err.h"

esp_err_t rest_common_get_handler(httpd_req_t *req);

#endif // __FRONT_END_HANDLER_H__