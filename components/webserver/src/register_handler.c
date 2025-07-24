#include <register_handler.h>
#include <webserver.h>
#include <cJSON.h>
#include <credential_store.h>
#include "esp_log.h"


const char *REGISTER_HANDLER_TAG = "REGISTER_HANDLER";

esp_err_t register_handler(httpd_req_t *req){
    if (req == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    ESP_LOGI(REGISTER_HANDLER_TAG, "Handling registration request for URI: %s", req->uri);
    int total_len = req->content_len;
    if (total_len <= 0 || total_len >= SCRATCH_BUFSIZE) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
        return ESP_FAIL; // Handle invalid content length
    }
    ESP_LOGI(REGISTER_HANDLER_TAG, "Total length of request body: %d", total_len);
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
    ESP_LOGI(REGISTER_HANDLER_TAG, "Received registration data: %s", buf);
    cJSON *root = cJSON_Parse(buf);
    if (root == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON format");
        return ESP_FAIL; // Handle JSON parsing error
    }

    const char* username = cJSON_GetObjectItem(root, "username")->valuestring;
    const char* password1 = cJSON_GetObjectItem(root, "password1")->valuestring;
    const char* password2 = cJSON_GetObjectItem(root, "password2")->valuestring;

    if(strlen(username) == 0 || strlen(username) > CONFIG_MAX_USERNAME_LENGTH) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid username length");
        return ESP_FAIL; // Handle invalid username length
    }

    if(strlen(password1) == 0 || strlen(password1) > CONFIG_MAX_PASSWORD_LENGTH) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid password length");
        return ESP_ERR_INVALID_SIZE; // Handle invalid password length
    }

    if(strlen(password2) == 0 || strlen(password2) > CONFIG_MAX_PASSWORD_LENGTH) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid password length");
        return ESP_FAIL; // Handle invalid password length
    }
    if(strncmp(password1,password2,CONFIG_MAX_PASSWORD_LENGTH)){
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Passwords do not match");
        return ESP_FAIL; // Handle password mismatch
    }

    

    // Here you would typically handle the registration logic,
    // such as saving the username and password to a database or file.
    esp_err_t err = auth_store_set(username, password1);
    if (err != ESP_OK) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to store credentials");
        return ESP_FAIL; // Handle storage failure   
    }
    cJSON_Delete(root);
    
    httpd_resp_sendstr(req, "Registration successful");
    return ESP_OK; // Return success
}