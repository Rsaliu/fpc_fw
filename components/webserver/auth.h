#ifndef __WEB_SERVER_AUTH__
#define __WEB_SERVER_AUTH__
#include "esp_http_server.h"
#include "esp_err.h"
#include <session.h>

esp_err_t auth_handler(httpd_req_t *req, session_t **session);
#endif // __WEB_SERVER_AUTH__