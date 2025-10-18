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
    file_deinit();
}


// TEST_CASE("file_handler_test", "test_file_create_and_write")
// {
//     fileHandlersetUp();
//     const char *json_data = "{\"name\":\"ESP32\",\"id\":1}";
//     TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data));

//     size_t size = 0;
//     TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size));
//     TEST_ASSERT_GREATER_THAN_UINT32(0, size);
//     fileHandlertearDown();
// }


TEST_CASE("file_handler_test", "test_file_create_and_write")
{
    fileHandlersetUp();
    const char *json_data = "{\"name\":\"ESP32\",\"id\":1}";
    size_t expected_size = strlen(json_data);
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size));
    TEST_ASSERT_EQUAL_UINT32(expected_size, size);
    fileHandlertearDown();
}

// TEST_CASE("file_handler_test", "test_file_append")
// {
//     fileHandlersetUp();
//     TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, "{\"sensor\":\"temp\"}\n"));
//     TEST_ASSERT_EQUAL(ESP_OK, file_append(TEST_FILE, "{\"value\":25}\n"));

//     size_t size_before = 0, size_after = 0;
//     TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size_before));

//     TEST_ASSERT_EQUAL(ESP_OK, file_append(TEST_FILE, "{\"value\":30}\n"));
//     TEST_ASSERT_EQUAL(ESP_OK, file_get_size(TEST_FILE, &size_after));

//     TEST_ASSERT_GREATER_THAN_UINT32(size_before, size_after);
//     fileHandlertearDown();
// }

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
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, json_data));

    char *buffer = NULL;
    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_OK, file_read(TEST_FILE, &buffer, &size));
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

    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, "{\"rename\":true}\n"));

    TEST_ASSERT_EQUAL(ESP_OK, file_rename(TEST_FILE, RENAMED_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, file_get_size(TEST_FILE, &size));   
    TEST_ASSERT_EQUAL(ESP_OK, file_get_size(RENAMED_FILE, &size));
    
    fileHandlertearDown();
}


TEST_CASE("file_handler_test", "test_file_delete")
{
    fileHandlersetUp();
    TEST_ASSERT_EQUAL(ESP_OK, file_write(TEST_FILE, "{\"delete\":true}\n"));
    TEST_ASSERT_EQUAL(ESP_OK, file_delete(TEST_FILE));

    size_t size = 0;
    TEST_ASSERT_EQUAL(ESP_FAIL, file_get_size(TEST_FILE, &size));
    fileHandlertearDown();
}

// TEST_CASE("file_handler_test", "test_list_files")
// {
//     fileHandlersetUp();
//     TEST_ASSERT_EQUAL(ESP_OK, file_write("/spiffs/file1.json", "{\"file\":1}\n"));
//     TEST_ASSERT_EQUAL(ESP_OK, file_write("/spiffs/file2.json", "{\"file\":2}\n"));

//     file_list();
//     fileHandlertearDown();
// }

TEST_CASE("file_handler_test", "test_list_files")
{
    fileHandlersetUp();
    const char *data1 = "{\"file\":1}\n";
    const char *data2 = "{\"file\":2}\n";
    size_t len1 = strlen(data1);
    size_t len2 = strlen(data2);

    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file("/spiffs/file1.json", data1, len1));
    TEST_ASSERT_EQUAL(ESP_OK, spiffs_write_file("/spiffs/file2.json", data2, len2));

    // List files (logs the directory contents)
    spiffs_list_files();

    // Verify files were created and are listable by checking existence and size
    struct stat st;
    TEST_ASSERT_EQUAL(0, stat("/spiffs/file1.json", &st));
    TEST_ASSERT_EQUAL_UINT32(len1, st.st_size);

    TEST_ASSERT_EQUAL(0, stat("/spiffs/file2.json", &st));
    TEST_ASSERT_EQUAL_UINT32(len2, st.st_size);

    // Optional: Assert total files (if you have a count function; otherwise, manual readdir count)
    // size_t file_count = spiffs_get_file_count("/spiffs");
    // TEST_ASSERT_EQUAL_UINT32(2, file_count);

    fileHandlertearDown();
}