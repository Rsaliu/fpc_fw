#include "unity.h"
#include "file_handler.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "TEST_FILE_HANDLER";

static const char *TEST_FILE = "/spiffs/test.json";
static const char *RENAMED_FILE = "/spiffs/test_renamed.json";

void fileHandlersetUp(void) {
    file_init();
}

void fileHandlertearDown(void) {
    file_deinit();
}

TEST_CASE("file_handler_test", "test_file_create_and_write")
{
    fileHandlersetUp();
    const char *json_data = "{\"name\":\"ESP32\",\"id\":1}";
    size_t expected_size = strlen(json_data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data, expected_size));  // Fixed: Added size arg

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
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, data, data_size));  // Fixed: Added size arg

    TEST_ASSERT_EQUAL(ESP_OK, file_rename(TEST_FILE, RENAMED_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, file_get_size(TEST_FILE, &size));   
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(RENAMED_FILE, &size));
    TEST_ASSERT_EQUAL_UINT32(data_size, size);  // Added: Verify size post-rename
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_delete")
{
    fileHandlersetUp();
    const char *data = "{\"delete\":true}\n";
    size_t data_size = strlen(data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, data, data_size));  // Fixed: Added size arg
    TEST_ASSERT_EQUAL(ESP_OK, file_delete(TEST_FILE));  // Fixed: file_delete (not file_delete_file)

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

    TEST_ASSERT_EQUAL(ESP_OK, file_write("/spiffs/file1.json", data1, len1));  // Fixed: file_write (not file_write_file)
    TEST_ASSERT_EQUAL(ESP_OK, file_write("/spiffs/file2.json", data2, len2));


    file_list_files("/spiffs");

    
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size("/spiffs/file1.json", &size1));  // Use API for consistency
    TEST_ASSERT_EQUAL_UINT32(len1, size1);

    TEST_ASSERT_EQUAL(ESP_OK, file_get_size("/spiffs/file2.json", &size2));
    TEST_ASSERT_EQUAL_UINT32(len2, size2);
    fileHandlertearDown();
}