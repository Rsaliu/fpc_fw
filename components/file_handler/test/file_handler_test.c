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

    spiffs_init();
}



void fileHandlertearDown(void) {
    spiffs_deinit();
}


TEST_CASE("file_handler_test", "test_file_create_and_write")
{
    fileHandlersetUp();
    const char *json_data = "{\"name\":\"ESP32\",\"id\":1}";
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file(TEST_FILE, json_data));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_get_file_size(TEST_FILE, &size));
    TEST_ASSERT_GREATER_THAN_UINT32(0, size);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_append")
{
    fileHandlersetUp();
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file(TEST_FILE, "{\"sensor\":\"temp\"}\n"));
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_append_file(TEST_FILE, "{\"value\":25}\n"));

    size_t size_before = 0, size_after = 0;
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_get_file_size(TEST_FILE, &size_before));

    TEST_ASSERT_EQUAL(ESP_OK, spiffs_append_file(TEST_FILE, "{\"value\":30}\n"));
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_get_file_size(TEST_FILE, &size_after));

    TEST_ASSERT_GREATER_THAN_UINT32(size_before, size_after);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_read")
{
    fileHandlersetUp();
    const char *json_data = "{\"read_test\":true}";
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file(TEST_FILE, json_data));

    char *buffer = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_read_file(TEST_FILE, &buffer, &size));
    TEST_ASSERT_NOT_NULL(buffer);

    TEST_ASSERT_EQUAL_STRING(json_data, buffer);
    free(buffer);
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_file_rename")
{
    fileHandlersetUp();

    spiffs_delete_file(TEST_FILE);
    spiffs_delete_file(RENAMED_FILE);

    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file(TEST_FILE, "{\"rename\":true}\n"));

    TEST_ASSERT_EQUAL(ESP_OK, spiffs_rename_file(TEST_FILE, RENAMED_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, spiffs_get_file_size(TEST_FILE, &size));   
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_get_file_size(RENAMED_FILE, &size));
    
    fileHandlertearDown();
}


TEST_CASE("file_handler_test", "test_file_delete")
{
    fileHandlersetUp();
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file(TEST_FILE, "{\"delete\":true}\n"));
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_delete_file(TEST_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, spiffs_get_file_size(TEST_FILE, &size));
    fileHandlertearDown();
}

TEST_CASE("file_handler_test", "test_list_files")
{
    fileHandlersetUp();
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file("/spiffs/file1.json", "{\"file\":1}\n"));
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file("/spiffs/file2.json", "{\"file\":2}\n"));

    spiffs_list_files();
    fileHandlertearDown();
}
