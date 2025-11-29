#include "unity.h"
#include "file_handler.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>
#include <stdlib.h>


#define USE_SPIFFS


esp_err_t spiffs_init(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE("SPIFFS", "Failed to get SPIFFS partition info (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI("SPIFFS", "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

esp_err_t spiffs_deinit(void) {
    ESP_LOGI("SPIFFS", "SPIFFS to be unmounted");
    return esp_vfs_spiffs_unregister(NULL);
}

#define BASE_PATH "/spiffs/"

#define CONCAT_PATH(file) BASE_PATH file

static const char *TEST_FILE = CONCAT_PATH("test.json");
static const char *RENAMED_FILE = CONCAT_PATH("test_renamed.json");

void fileHandlersetUp(void) {
    file_register_fs(spiffs_init, spiffs_deinit);
    file_init();
}

void fileHandlertearDown(void) {
    esp_err_t  err = file_deinit();
    if(err != ESP_OK){
        exit(1);
    }
    file_delete(TEST_FILE);
    file_delete(RENAMED_FILE);
}

TEST_CASE("file_handler_test", "test_file_create_and_write")
{
    fileHandlersetUp();
    const char *json_data = "{\"name\":\"ESP32\",\"id\":1}";
    size_t expected_size = strlen(json_data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data, expected_size));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size));
    TEST_ASSERT_EQUAL_UINT32(expected_size, size);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_append")
{
    fileHandlersetUp();
    const char *initial_data = "{\"sensor\":\"temp\"}\n";
    const char *first_append = "{\"value\":25}\n";
    const char *second_append = "{\"value\":30}\n";
    size_t initial_len = strlen(initial_data);
    size_t append_len = strlen(first_append);
    size_t second_append_len = strlen(second_append);

    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, initial_data, initial_len));
    TEST_ASSERT_EQUAL(ESP_OK, file_append(TEST_FILE, first_append, append_len));

    size_t size_before = 0;
    size_t expected_before = initial_len + append_len;
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size_before));
    TEST_ASSERT_EQUAL_UINT32(expected_before, size_before);

    TEST_ASSERT_EQUAL(ESP_OK, file_append(TEST_FILE, second_append, second_append_len));
    size_t size_after = 0;
    size_t expected_after = expected_before + second_append_len;
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size_after));
    TEST_ASSERT_EQUAL_UINT32(expected_after, size_after);

    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_read")
{
    fileHandlersetUp();
    const char *json_data = "{\"read_test\":true}";
    size_t data_size = strlen(json_data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data, data_size));

    char *buffer = malloc(data_size + 1);
    TEST_ASSERT_NOT_NULL(buffer);
    size_t read_size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, file_read(TEST_FILE, buffer, data_size + 1, &read_size));
    TEST_ASSERT_EQUAL_UINT32(data_size, read_size);

    TEST_ASSERT_EQUAL_STRING(json_data, buffer);
    free(buffer);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_rename")
{
    fileHandlersetUp();

    file_delete(TEST_FILE); 
    file_delete(RENAMED_FILE);

    const char *data = "{\"rename\":true}\n";
    size_t data_size = strlen(data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, data, data_size));

    TEST_ASSERT_EQUAL(ESP_OK, file_rename(TEST_FILE, RENAMED_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, file_get_size(TEST_FILE, &size));   
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(RENAMED_FILE, &size));
    TEST_ASSERT_EQUAL_UINT32(data_size, size);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_delete")
{
    fileHandlersetUp();
    const char *data = "{\"delete\":true}\n";
    size_t data_size = strlen(data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, data, data_size));
    TEST_ASSERT_EQUAL(ESP_OK, file_delete(TEST_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, file_get_size(TEST_FILE, &size));
    
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_list_files")
{
    fileHandlersetUp();
    const char *data1 = "{\"file\":1}\n";
    const char *data2 = "{\"file\":2}\n";
    size_t len1 = strlen(data1);
    size_t len2 = strlen(data2);

    TEST_ASSERT_EQUAL(ESP_OK, file_write(CONCAT_PATH("file1.json"), data1, len1));
    TEST_ASSERT_EQUAL(ESP_OK, file_write(CONCAT_PATH("file2.json"), data2, len2));

    file_list_files(BASE_PATH);
}