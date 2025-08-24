#include <config_handler.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <cJSON.h>
#include <session.h>  
#include <webserver_utils.h>  
#include <auth.h>
const char *CONFIG_HANDLER_TAG = "CONFIG_HANDLER";
static error_type_t save_config_to_file(cJSON *config,char* config_file_path);

esp_err_t set_config_handler(httpd_req_t *req){
    esp_err_t err;
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }

    ESP_LOGI(CONFIG_HANDLER_TAG, "Handling config request for URI: %s", req->uri);
    rest_server_context_t * context = (rest_server_context_t *)req->user_ctx;
    char *buf = (char *)(context->scratch);
    err = retrieve_http_request_body(req, buf, SCRATCH_BUFSIZE);
    inject_cors_options(req); // Set CORS headers for the request
    if (err != ESP_OK) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to retrieve HTTP request body: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to retrieve request body",buf,SCRATCH_BUFSIZE)
            );
        return err; // Handle request body retrieval failure
    }
    ESP_LOGI(CONFIG_HANDLER_TAG, "Received config data: %s", buf);
    session_t *session = NULL;
    err = auth_handler(req,&session);
    if (err != ESP_OK) {
        return err; // Handle authentication failure
    }
    cJSON *root = cJSON_Parse(buf);
    if (root == NULL) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid JSON format",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle JSON parsing error
    }


    //****  will confirm content of json later. */

    // store json file in flash

    if (context == NULL) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("context or base path is null",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_NO_MEM; // Handle null base path
    }
    char *base_path = context->base_path;
    char *config_file_path = context->config_file_path;
    if (base_path == NULL || config_file_path == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Base path or config file path is null");
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Base path or config file path is null",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_NO_MEM; // Handle null base path or config file path 
    }
    ESP_LOGI(CONFIG_HANDLER_TAG, "base path: %s", base_path);
    ESP_LOGI(CONFIG_HANDLER_TAG, "config path: %s", config_file_path);
    // Allocate memory for the config file path
    char full_config_path[50];
    snprintf(full_config_path,50, "%s/%s",base_path , config_file_path);
    ESP_LOGI(CONFIG_HANDLER_TAG, "Storing configuration to file: %s", full_config_path);

    error_type_t save_result = save_config_to_file(root,full_config_path);
    if (save_result != SYSTEM_OK) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to save configuration: %d", save_result);
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to save configuration",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle configuration saving failure
    }
    cJSON_Delete(root); // Free the cJSON object after saving
    // Send a response indicating success
    httpd_resp_set_status(req, HTTPD_200);
    err = httpd_resp_sendstr(req,
        create_response_json("Configuration saved successfully",buf,SCRATCH_BUFSIZE)
        );
    if(err != ESP_OK) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return err; // Handle response sending failure
    }
    ESP_LOGI(CONFIG_HANDLER_TAG,"config saved successfully");
    return ESP_OK; // Return success
}

static error_type_t save_config_to_file(cJSON *config,char* config_file_path) {
    if (config == NULL || config_file_path == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Invalid parameters for saving configuration");
        return SYSTEM_INVALID_PARAMETER; // Handle null parameters
    }
    // Convert cJSON object to string
    char *config_str = cJSON_PrintUnformatted(config);
    if (config_str == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to convert config to string");
        return SYSTEM_OPERATION_FAILED; // Handle conversion failure
    }

    // Open the file for writing
    FILE *file = fopen(config_file_path, "w");
    if (file == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to open config file for writing");
        cJSON_free(config_str);
        return SYSTEM_OPERATION_FAILED; // Handle file open failure
    }

    // Write the config string to the file
    fprintf(file, "%s", config_str);
    fclose(file);
    cJSON_free(config_str); // Free the allocated string

    ESP_LOGI(CONFIG_HANDLER_TAG, "Configuration saved successfully to %s", config_file_path);
    return SYSTEM_OK; // Return success
}

esp_err_t get_config_handler(httpd_req_t *req){ 
    if (req == NULL) {
        //httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "null request");
        return ESP_ERR_NO_MEM; // Handle null request
    }
    inject_cors_options(req); // Set CORS headers for the request
    rest_server_context_t * context = (rest_server_context_t *)req->user_ctx;
    char *buf = (char *)(context->scratch);
    ESP_LOGI(CONFIG_HANDLER_TAG, "Handling config request for URI: %s", req->uri);
    esp_err_t err;
    session_t *session = NULL;
    err = auth_handler(req,&session);
    if (err != ESP_OK) {
        return err; // Handle authentication failure
    }
    char *base_path = context->base_path;
    char *config_file_path = context->config_file_path;
    if (base_path == NULL || config_file_path == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Base path or config file path is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Base path or config file path is null",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_NO_MEM; // Handle null base path or config file path 
    }
    ESP_LOGI(CONFIG_HANDLER_TAG, "base path: %s", base_path);
    // Allocate memory for the config file path
    char full_config_path[50];
    snprintf(full_config_path,50, "%s/%s",base_path , config_file_path);
    ESP_LOGI(CONFIG_HANDLER_TAG, "Storing configuration to file: %s", full_config_path);
    FILE *file = fopen(full_config_path, "r");
    if (file == NULL) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to open config file");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to open config file",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle file open failure
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0 || file_size >= SCRATCH_BUFSIZE) {
        fclose(file);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
            create_response_json("Invalid file size",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle invalid file size
    }
    char * read_buffer = (context->scratch);
    if (read_buffer == NULL) {
        fclose(file);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Scratch buffer is null",buf,SCRATCH_BUFSIZE)
            );
        return ESP_ERR_NO_MEM; // Handle null scratch buffer
    }
    size_t read_size = fread(read_buffer, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            create_response_json("Failed to read config file",buf,SCRATCH_BUFSIZE)
            );
        return ESP_FAIL; // Handle read failure
    }
    read_buffer[read_size] = '\0'; // Null-terminate the string
    ESP_LOGI(CONFIG_HANDLER_TAG, "Read configuration data: %s", read_buffer);
    // Send the configuration as a response
    httpd_resp_set_status(req, HTTPD_200);
    err = httpd_resp_sendstr(req,
        read_buffer);
    if (err != ESP_OK) {
        ESP_LOGE(CONFIG_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return err; // Handle response sending failure
    }
    ESP_LOGI(CONFIG_HANDLER_TAG, "Configuration retrieved successfully");
    return ESP_OK; // Return success
}