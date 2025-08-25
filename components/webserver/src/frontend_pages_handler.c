#include <frontend_pages_handler.h>
#include <webserver.h>
#include <webserver_utils.h>
const char *FRONTEND_HANDLER_TAG = "FRONTEND_HANDLER";

#include <esp_log.h>
#include <esp_http_server.h>

static esp_err_t read_file_content(const char* file_path, char* buffer, size_t buffer_size,size_t* read_size_out) {
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

    ESP_LOGI(FRONTEND_HANDLER_TAG,"file size: %d\n",read_size);
    *read_size_out = read_size;
    return ESP_OK; // Return success
}

static esp_err_t get_page(const char* file_path, httpd_req_t *req) {
    rest_server_context_t *context = (rest_server_context_t *)(req->user_ctx);
    if(context == NULL){
        ESP_LOGE(FRONTEND_HANDLER_TAG, "context is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "context is null");
        return ESP_ERR_NO_MEM; // Handle null base path or config file path     
    }
    char *base_path = context->base_path;
    if (base_path == NULL) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Base path is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Base path is null");
        return ESP_ERR_NO_MEM; // Handle null base path or config file path 
    }
    ESP_LOGI(FRONTEND_HANDLER_TAG, "base path: %s", base_path);
    char directory_name[ESP_VFS_PATH_MAX + 1];
    get_content_directory_name(file_path, directory_name, sizeof(directory_name));
    // Allocate memory for the file path
    char full_file_path[50];
    snprintf(full_file_path,50, "%s/%s%s",base_path,directory_name,file_path);
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Retrieving page from file: %s", full_file_path);
    char* read_buffer = (context->scratch);
    if (read_buffer == NULL) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Scratch buffer is null");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Scratch buffer is null");
        return ESP_ERR_NO_MEM; // Handle null scratch buffer
    }
    size_t read_size;
    esp_err_t err = read_file_content(full_file_path, read_buffer, SCRATCH_BUFSIZE,&read_size);
    if (err != ESP_OK) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to read page content");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read page content");
        return err; // Handle file read failure 
    }
    err = set_content_type_from_file(req, full_file_path);
    if (err != ESP_OK) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to set content type: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set content type");
        return err; // Handle content type setting failure
    }
  
    // Send the page as a response
    httpd_resp_set_status(req, HTTPD_200);
    err = httpd_resp_send(req, read_buffer,read_size);
    if (err != ESP_OK) {
        ESP_LOGE(FRONTEND_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
        return err; // Handle response sending failure
    }   
    ESP_LOGI(FRONTEND_HANDLER_TAG, "Page retrieved successfully");
    return ESP_OK; // Return success    
}


esp_err_t rest_common_get_handler(httpd_req_t *req){
  char filepath[30];

    if (req->uri[strlen(req->uri) - 1] == '/') {
        strncpy(filepath, "/home_ui.html", sizeof(filepath));
    } else {
        strncpy(filepath, req->uri, sizeof(filepath));
    }
    return get_page(filepath, req);
}