#include   <auth.h>
#include   <webserver.h>
#include <webserver_utils.h>
#include <session.h>
#include   <esp_log.h>
const char *AUTH_HANDLER_TAG = "AUTH_HANDLER";
esp_err_t auth_handler(httpd_req_t *req,session_t **session) {
    char cookie_hdr[128];
    char sid[65];
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    esp_err_t err = httpd_req_get_hdr_value_str(req, "Cookie", cookie_hdr, sizeof(cookie_hdr));
    if ( err != ESP_OK) {
        ESP_LOGE(AUTH_HANDLER_TAG, "Failed to get Cookie header %s",esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Cookie header not found");
        return ESP_FAIL; // Handle missing cookie header
    }
    ESP_LOGI(AUTH_HANDLER_TAG, "Cookie header: %s", cookie_hdr);
    if (parse_cookie(cookie_hdr, "SID", sid, sizeof(sid)) != ESP_OK) {
        ESP_LOGE(AUTH_HANDLER_TAG, "Session ID not found in Cookie header");
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Session ID not found in Cookie header");
        return ESP_FAIL; // Handle missing session ID
    }
    ESP_LOGI(AUTH_HANDLER_TAG, "Session ID: %s", sid);
    session_t *_session = find_session_by_token(sid);
    if (_session == NULL) {
        ESP_LOGI(AUTH_HANDLER_TAG, "Session is NULL");
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Unauthorized access");
        return ESP_FAIL; // Handle unauthorized access
    }
    // Log the request URI and session username
    ESP_LOGI(AUTH_HANDLER_TAG, "Session username: %s", _session->username);
    ESP_LOGI(AUTH_HANDLER_TAG, "Handling config request for URI: %s", req->uri);
    *session = _session; // Set the session pointer
    return ESP_OK; // Return success
}