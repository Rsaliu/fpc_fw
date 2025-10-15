#ifndef __WEB_SERVER_UTILS_H__
#define __WEB_SERVER_UTILS_H__
#include "esp_http_server.h"
#include "esp_err.h"
#include <stddef.h>


esp_err_t retrieve_http_request_body(httpd_req_t *req, char *buffer, size_t buffer_size);

void inject_cors_options(httpd_req_t *req);
esp_err_t parse_cookie(const char *cookie_hdr, const char *name, char *out, size_t out_len);
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath);
void get_content_directory_name(const char *filepath, char* buff, size_t buff_size);
const char *create_response_json(const char *message, char *buffer, size_t buffer_size);

#endif // __WEB_SERVER_UTILS_H__