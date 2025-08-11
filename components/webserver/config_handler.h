#ifndef __CONFIG_HANDLER_H__
#define __CONFIG_HANDLER_H__
#include "webserver.h"
#include "esp_http_server.h"
#include <common_headers.h>

esp_err_t set_config_handler(httpd_req_t *req);
esp_err_t get_config_handler(httpd_req_t *req);
#endif // __CONFIG_HANDLER_H__