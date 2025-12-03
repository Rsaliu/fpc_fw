#include <ads1115.h>
#include <unity.h>
#include <freertos/FreeRTOS.h>
QueueHandle_t overcurrent_event_queue;
bool done_flag = false;
SemaphoreHandle_t xMutex;
ads1115_t *ads1115_object = NULL;
    ads1115_config_t ads_config = {
    .i2c_port = I2C_NUM_1,                         // Use I2C port 0
    .sda_gpio = GPIO_NUM_21,                       // SDA pin
    .scl_gpio = GPIO_NUM_22,                       // SCL pin
    .alert_ready_pin = GPIO_NUM_34,                // ALERT/READY pin
    .frequency_hz = 100000,                        // I2C frequency
    .clock_source = I2C_CLK_SRC_DEFAULT,           // Default clock source
    .glitch_ignore_cnt = 0,                        // No glitch ignore
    .enable_pullup = 1,                            // Enable pull-up resistors
    .i2c_address = ADDR_GROUNDED,                  // Use grounded address
    .pga_mode = ADS1115_PGA_6_144V,   
    };


void ads1115_teardown();

void create_overcurrent_event_queue(){
    if(overcurrent_event_queue == NULL){
        overcurrent_event_queue = xQueueCreate(10, sizeof(overcurrent_queue_item_t));
    }
}

void comparator_callback(overcurrent_queue_item_t item){
    //printf("event timestamp: %ld\n", item.timestamp);
    xQueueSendToFrontFromISR(overcurrent_event_queue, &item, NULL);
}


void spool_overcurrent_event(){
    // get items from queue until empty
    overcurrent_queue_item_t item;
    uint8_t count = 0;
    while(1){
    while(xQueueReceive(overcurrent_event_queue, &item, pdMS_TO_TICKS(1000)) == pdTRUE){
        //printf("Overcurrent event received! Timestamp: %ld, Channel: %d\n", item.timestamp, item.channel);
        count++;
        ads1115_t* ads1115_object = (ads1115_t*)item.context;
        int16_t raw_value = 0;
        
        error_type_t err = ads1115_read_one_shot_with_channel(ads1115_object, &raw_value, (ads1115_input_channel_t)item.channel);
        if(err != SYSTEM_OK){
            printf("Error reading ADS1115 in callback: %d\n", err);
            continue;
        }
        TickType_t current_time = xTaskGetTickCount();
        TickType_t diff_time = current_time - item.timestamp;
        printf("ADS1115 read value in callback: %d\n", raw_value);
        printf("Error re-arming comparator in callback: %d\n", err);
        printf("Time since last event: %ld ticks\n", diff_time);
        printf("Comparator re-armed successfully in callback\n");
        err = ads1115_read_comparator_with_channel(ads1115_object, 3000, 2000, comparator_callback, item.callers_context, (ads1115_input_channel_t)item.channel);
        if(err != SYSTEM_OK){
            printf("Error re-arming comparator in callback: %d\n", err);
            continue;
        }
        if(count >= 10){
            xSemaphoreTake( xMutex, portMAX_DELAY ); 
            done_flag = true;
            xSemaphoreGive( xMutex );
            // delete task after 10 events
            vTaskDelete(NULL);
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
}
void ads1115_setup(){
    create_overcurrent_event_queue();
    ads1115_teardown();
    ads1115_object = ads1115_create(&ads_config);
    TEST_ASSERT_NOT_NULL(ads1115_object);
}



void ads1115_teardown(){
    if (ads1115_object != NULL) {
        printf("Tearing down ADS1115 instance");
        ads1115_destroy(&ads1115_object);
    }
}

TEST_CASE("ads1115_test", "test_ads1115_create") {
    ads1115_setup();
    TEST_ASSERT_NOT_NULL(ads1115_object);
    ads1115_teardown();
}

TEST_CASE("ads1115_test", "test_ads1115_init") {
    ads1115_setup();
    printf("about to init ADS1115\n");
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ads1115_teardown();
}

TEST_CASE("ads1115_test", "test_ads1115_read_with_channel") {
    ads1115_setup();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("about to read ADC value\n");
    int16_t raw_value = 0;
    result = ads1115_read_one_shot_with_channel(ads1115_object, &raw_value,ADS1115_CHANNEL_0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("Raw ADC value: %d\n", raw_value);

    ads1115_teardown();
}

TEST_CASE("ads1115_test", "test_ads1115_read") {
    ads1115_setup();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("about to read ADC value\n");
    int16_t raw_value = 0;
    result = ads1115_set_read_channel(ads1115_object, ADS1115_CHANNEL_0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = ads1115_read_one_shot(ads1115_object, &raw_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("Raw ADC value: %d\n", raw_value);
    ads1115_teardown();
}


TaskHandle_t spoolTaskHandle;

void startPoolingTask(){
    xTaskCreate(
        (TaskFunction_t)spool_overcurrent_event,   // Function that implements the task.
        "SpoolOvercurrentEvent",        // Text name for the task.
        4096,                           // Stack size in bytes, not words.
        NULL,                           // Parameter passed into the task.
        tskIDLE_PRIORITY,               // Priority at which the task is created.
        &spoolTaskHandle);              // Used to pass out the created task's handle.
}

int dummy= 10;

// only run this test if ads1115 alert pin is connected to MCU and overcurrent condition can be simulated
#if 0
TEST_CASE("ads1115_test", "test_ads1115_read_comparator") {
    ads1115_setup();
     xMutex = xSemaphoreCreateMutex();
    startPoolingTask();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("about to read ADC value\n");
    result = ads1115_set_read_channel(ads1115_object, ADS1115_CHANNEL_0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    result = ads1115_read_comparator(ads1115_object, 3000, 2000, comparator_callback, (void*)&dummy);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    while(1){
        xSemaphoreTake( xMutex, portMAX_DELAY ); 
        if(done_flag){
            xSemaphoreGive( xMutex );
            break;
        }
        xSemaphoreGive( xMutex );
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ads1115_teardown();
}
#endif

TEST_CASE("ads1115_test", "test_ads1115_destroy") {
    ads1115_setup();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = ads1115_destroy(&ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(ads1115_object); // ADC should be NULL after destruction
}

TEST_CASE("ads1115_test", "test_ads1115_deinit") {
    ads1115_setup();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = ads1115_deinit(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ads1115_teardown();
}