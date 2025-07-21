#include <ads1115.h>
#include <unity.h>

ads1115_t *ads1115_object = NULL;
    ads1115_config_t ads_config = {
        .i2c_port = I2C_NUM_1, // Use I2C port 0
        .sda_gpio = GPIO_NUM_33, // SDA pin
        .scl_gpio = GPIO_NUM_32, // SCL pin
        .frequency_hz = 100000, // I2C frequency
        .clock_source = I2C_CLK_SRC_DEFAULT, // Default clock source
        .glitch_ignore_cnt = 0, // No glitch ignore
        .enable_pullup = 1, // Enable pull-up resistors
        .i2c_address = ADDR_GROUNDED, // Use grounded address
        .pga_mode = ADS1115_PGA_6_144V // Set PGA mode to 6.144V
    };
void ads1115_teardown();
void ads1115_setup(){

    ads1115_teardown();
    ads1115_object = ads1115_create(&ads_config);
    TEST_ASSERT_NOT_NULL(ads1115_object);
}

void ads1115_teardown(){
    if (ads1115_object != NULL) {
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
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    ads1115_teardown();
}

TEST_CASE("ads1115_test", "test_ads1115_read") {
    ads1115_setup();
    error_type_t result = ads1115_init(ads1115_object);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("about to read ADC value\n");
    int16_t raw_value = 0;
    result = ads1115_read(ads1115_object, &raw_value,ADS1115_CHANNEL_0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("Raw ADC value: %d\n", raw_value);

    ads1115_teardown();
}

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