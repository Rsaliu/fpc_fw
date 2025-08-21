#include <frontend_pages_handler.h>
#include <webserver.h>
const char *FRONTEND_HANDLER_TAG = "FRONTEND_HANDLER";
const char* root_file_path = "index.html"; // Path to the root HTML file
const char* login_file_path = "login.html"; // Path to the login HTML file
const char* register_file_path = "register.html"; // Path to the register HTML file
const char* config_file_path = "config.html"; // Path to the config HTML file

#include <esp_log.h>
#include <esp_http_server.h>

static esp_err_t read_file_content(const char* file_path, char* buffer, size_t buffer_size) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to open file: %s", file_path);
        return ESP_FAIL; // Handle file open failure
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0 || file_size >= buffer_size) {
        fclose(file);
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Invalid file size: %ld", file_size);
        return ESP_FAIL; // Handle invalid file size
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to read file content");
        return ESP_FAIL; // Handle read failure
    }

    buffer[read_size] = '\0'; // Null-terminate the string
    return ESP_OK; // Return success
}

static esp_err_t get_page(const char* file_path, httpd_req_t *req) {
    rest_server_context_t *context = (rest_server_context_t *)(req->user_ctx);
    if(context == NULL){
        ESP_LOGE(FRONTEND_HANDLER_TAG, "context is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "context is null");
        return ESP_ERR_NO_MEM; // Handle null base path or config file path     
    }
    char *base_path = "";
    if (base_path == NULL) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Base path is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Base path is null");
        return ESP_ERR_NO_MEM; // Handle null base path or config file path 
    }
    ESP_LOGI(FRONTEND_HANDLER_TAG, "base path: %s", base_path);
    // Allocate memory for the file path
    char full_file_path[50];
    snprintf(full_file_path,50, "%s/%s",base_path , file_path);
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Retrieving page from file: %s", full_file_path);
    char* read_buffer = (context->scratch);
    if (read_buffer == NULL) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Scratch buffer is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scratch buffer is null");
        return ESP_ERR_NO_MEM; // Handle null scratch buffer
    }
    esp_err_t err = read_file_content(full_file_path, read_buffer, SCRATCH_BUFSIZE);
    if (err != ESP_OK) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to read page content");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read page content");
        return err; // Handle file read failure 
    }
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Read page data: %s", read_buffer);
    // Set the content type to HTML
    httpd_resp_set_type(req, "text/html");
    // Send the page as a response
    httpd_resp_set_status(req, HTTPD_200);
    err = httpd_resp_sendstr(req, read_buffer);
    if (err != ESP_OK) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return err; // Handle response sending failure
    }   
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Page retrieved successfully");
    return ESP_OK; // Return success    
}


esp_err_t root_page_handler(httpd_req_t *req){
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Handling request for root page");
    return get_page(root_file_path, req);
}

esp_err_t login_page_handler(httpd_req_t *req){
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Handling request for login page");
    return get_page(login_file_path, req);
}
esp_err_t register_page_handler(httpd_req_t *req){
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Handling request for register page");
    return get_page(register_file_path, req);   
}
esp_err_t config_page_handler(httpd_req_t *req){
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Handling request for config page");
    return get_page(config_file_path, req);
}