#include <login_handler.h>
#include <webserver.h>
#include <cJSON.h>
#include <session.h>
#include <credential_store.h>
#include <esp_log.h>
#include <webserver_utils.h>

const char *LOGIN_HANDLER_TAG = "LOGIN_HANDLER";

esp_err_t login_handler(httpd_req_t *req){
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    inject_cors_options(req); // Set CORS headers for the request
    rest_server_context_t * context = (rest_server_context_t *)req->user_ctx;
    char *buf = (char *)(context->scratch);
    esp_err_t err = retrieve_http_request_body(req, buf, SCRATCH_BUFSIZE);
    if (err != ESP_OK) {
        ESP_LOGE(LOGIN_HANDLER_TAG, "Failed to retrieve HTTP request body: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to retrieve request body");
        return err; // Handle request body retrieval failure
    }

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
    // check if the user is already logged in
    session_t *existing_session = find_session_by_username(username);
    if (existing_session != NULL) {
        // User is already logged in, return an error
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_sendstr(req, "User already logged in");
        ESP_LOGI(LOGIN_HANDLER_TAG, "User already logged in");
        cJSON_Delete(root);
        return ESP_OK; // Handle already logged in user
    }
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
    err = httpd_resp_sendstr(req, "Login successful");
    if(err != ESP_OK) {
        ESP_LOGE(LOGIN_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return ESP_FAIL; // Handle response sending failure
    }
    cJSON_Delete(root);
    return ESP_OK; // Return success   
}