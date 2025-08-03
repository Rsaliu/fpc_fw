#ifndef __WEBSERVER_RESET_HANDLER_H__
#define __WEBSERVER_RESET_HANDLER_H__

#include "esp_http_server.h"
#include "esp_err.h"

esp_err_t reset_handler(httpd_req_t *req);

#endif // __WEBSERVER_RESET_HANDLER_H__