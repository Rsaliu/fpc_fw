#include <login_handler.h>
#include <webserver.h>
#include <cJSON.h>
#include <session.h>
#include <credential_store.h>
#include <esp_log.h>

const char *LOGIN_HANDLER_TAG = "LOGIN_HANDLER";

esp_err_t login_handler(httpd_req_t *req){
    if (req == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = (char *)(req->user_ctx);
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    const char* username = cJSON_GetObjectItem(root, "username")->valuestring;
    const char* password = cJSON_GetObjectItem(root, "password")->valuestring;
    if(strlen(username) == 0 || strlen(username) > LOGIN_MAX_USERNAME_LENGTH) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "username has invalid length");
        cJSON_Delete(root);
        return ESP_FAIL; // Handle invalid username length
    }

    if(strlen(password) == 0 || strlen(password) > LOGIN_MAX_PASSWORD_LENGTH) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "password has invalid length");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    if (!username || !password) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "username or password is null");
        cJSON_Delete(root);
        return ESP_FAIL; // Handle null username or password
    }
    ESP_LOGI(LOGIN_HANDLER_TAG, "username: %s and %s password", username,password);

    if (!auth_store_check(username, password)) {
        httpd_resp_send_err(req, HTTPD_401_UNAUTHORIZED, "Invalid username or password");
        cJSON_Delete(root);
        return ESP_FAIL; // Handle invalid credentials
    }
    cJSON_Delete(root);
    //set the session context to the username for further requests

    session_t *s = create_session(username);
    if (s == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create session");
        return ESP_FAIL; // Handle session creation failure
    }
    ESP_LOGI(LOGIN_HANDLER_TAG, "Session created for user: %s", username);
    set_session_cookie(req, s);
    // If authentication is successful, send a success response
    httpd_resp_set_status(req, HTTPD_200);
    ESP_LOGI(LOGIN_HANDLER_TAG, "User %s logged in successfully", username);
    esp_err_t err = httpd_resp_sendstr(req, "Login successful");
    if(err != ESP_OK) {
        ESP_LOGE(LOGIN_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return ESP_FAIL; // Handle response sending failure
    }
    return ESP_OK; // Return success   
}