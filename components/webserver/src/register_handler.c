#include <register_handler.h>
#include <webserver.h>
#include <cJSON.h>
#include <credential_store.h>
#include "esp_log.h"
#include <webserver_utils.h>
#include <esp_http_server.h>


const char *REGISTER_HANDLER_TAG = "REGISTER_HANDLER";

esp_err_t register_handler(httpd_req_t *req){
    esp_err_t err;
    bool result;
    err  = get_user_registered_flag(&result);
    if (req == NULL) {
        return ESP_ERR_NO_MEM; // Handle null request
    }
    inject_cors_options(req); // Set CORS headers for the request
    rest_server_context_t * context = (rest_server_context_t *)req->user_ctx;
    char *buf = (char *)(context->scratch);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to check user registration status",buf,SCRATCH_BUFSIZE)
            );
        return err; // Handle error checking user registration status
    }
    if (result) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("User already registered",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle already registered user
    }

    ESP_LOGI(REGISTER_HANDLER_TAG, "Handling registration request for URI: %s", req->uri);
    err = retrieve_http_request_body(req, buf, SCRATCH_BUFSIZE);
    if (err != ESP_OK) {
        ESP_LOGE(REGISTER_HANDLER_TAG, "Failed to retrieve HTTP request body: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to retrieve request body",buf,SCRATCH_BUFSIZE)
            );
        return err; // Handle request body retrieval failure
    }
    ESP_LOGI(REGISTER_HANDLER_TAG, "Received registration data: %s", buf);
    cJSON *root = cJSON_Parse(buf);
    if (root == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid JSON format",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle JSON parsing error
    }

    const char* username = cJSON_GetObjectItem(root, "username")->valuestring;
    const char* password1 = cJSON_GetObjectItem(root, "password1")->valuestring;
    const char* password2 = cJSON_GetObjectItem(root, "password2")->valuestring;

    if(strlen(username) == 0 || strlen(username) > CONFIG_MAX_USERNAME_LENGTH) {
        cJSON_Delete(root);
        
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid username length",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle invalid username length
    }

    if(strlen(password1) == 0 || strlen(password1) > CONFIG_MAX_PASSWORD_LENGTH) {
        cJSON_Delete(root);
        
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid password length",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_INVALID_SIZE; // Handle invalid password length
    }

    if(strlen(password2) == 0 || strlen(password2) > CONFIG_MAX_PASSWORD_LENGTH) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid password length",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle invalid password length
    }
    if(strncmp(password1,password2,CONFIG_MAX_PASSWORD_LENGTH)){
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Passwords do not match",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle password mismatch
    }

    

    // Here you would typically handle the registration logic,
    // such as saving the username and password to a database or file.
     err = auth_store_set(username, password1);
    if (err != ESP_OK) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to store credentials",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle storage failure   
    }
    cJSON_Delete(root);

    err = set_user_registered_flag();
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to set user registration flag",buf,SCRATCH_BUFSIZE)
            );
        return err; // Handle error setting user registration flag
    }
    
    httpd_resp_sendstr(req,
         create_response_json("Registration successful",buf,SCRATCH_BUFSIZE)
        );
    return ESP_OK; // Return success
}