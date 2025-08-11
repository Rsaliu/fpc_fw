#include <webserver_utils.h>
#include "esp_http_server.h"
#include "esp_log.h"

esp_err_t retrieve_http_request_body(httpd_req_t *req, char *buffer, size_t buffer_size) {
    if (req == NULL || buffer == NULL || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG; // Handle null or invalid arguments
    }

    int cur_len = 0;
    int received = 0;
    int total_len = req->content_len;
    if (total_len > buffer_size - 1) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_ERR_INVALID_SIZE;
    }

    while (cur_len < total_len) {
        received = httpd_req_recv(req, buffer + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buffer[total_len] = '\0';
    return ESP_OK; // Successfully retrieved request body
}

void inject_cors_options(httpd_req_t *req) {
    if (req == NULL) {
        ESP_LOGE("WEB_SERVER_UTILS", "Received null request");
        return; // Handle null request
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
}

esp_err_t parse_cookie(const char *cookie_hdr, const char *name, char *out, size_t out_len)
{
    // Make a mutable copy
    char buf[128];
    strncpy(buf, cookie_hdr, sizeof(buf));
    buf[sizeof(buf)-1] = 0;

    char *saveptr = NULL;
    char *token = strtok_r(buf, ";", &saveptr);
    while (token) {
        // skip leading spaces
        while (*token == ' ') token++;
        size_t name_len = strlen(name);
        if (strncmp(token, name, name_len) == 0 && token[name_len] == '=') {
            const char *val = token + name_len + 1;
            strlcpy(out, val, out_len);
            return ESP_OK;
        }
        token = strtok_r(NULL, ";", &saveptr);
    }
    return ESP_ERR_NOT_FOUND;
}
