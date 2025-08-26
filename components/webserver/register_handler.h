#ifndef __REGISTER_HANDLER_H__
#define __REGISTER_HANDLER_H__
#include <common_headers.h>
#include <esp_http_server.h>

esp_err_t register_handler(httpd_req_t *req);

#endif // __REGISTER_HANDLER_H__