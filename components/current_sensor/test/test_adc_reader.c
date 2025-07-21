#include <adc_reader.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <unity.h>

adc_reader_t* adc_reader = NULL;
adc_reader_config_t config = {
    .adc_unit_id = ADC_UNIT_1,
    .adc_channel = ADC_CHANNEL_0,
    .adc_atten = ADC_ATTEN_DB_12, // Set attenuation to 12 dB
    .adc_bitwidth = ADC_BITWIDTH_12
};
void adcReaderSetUp(void) {
    // Set up code before each test
    adc_reader = adc_reader_create(&config);
}

void adcReaderTearDown(void) {
    // Clean up code after each test
    if (adc_reader != NULL) {
        adc_reader_destroy(&adc_reader);
    }
}

TEST_CASE("adc_reader_test", "test_adc_reader_create") {
    adcReaderSetUp();
    TEST_ASSERT_NOT_NULL(adc_reader);
    adcReaderTearDown();
}

TEST_CASE("adc_reader_test", "test_adc_reader_init") {
    adcReaderSetUp();
    error_type_t result = adc_reader_init(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    adcReaderTearDown();
}

TEST_CASE("adc_reader_test", "test_adc_reader_read") {
    adcReaderSetUp();
    error_type_t result = adc_reader_init(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    int raw_value = 0;
    result = adc_reader_read(adc_reader, &raw_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    printf("Raw ADC value: %d\n", raw_value);

    adcReaderTearDown();
}

TEST_CASE("adc_reader_test", "test_adc_reader_destroy") {
    adcReaderSetUp();
    error_type_t result = adc_reader_init(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = adc_reader_destroy(&adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_NULL(adc_reader); // ADC reader should be NULL after destruction
}

TEST_CASE("adc_reader_test", "test_adc_reader_deinit") {
    adcReaderSetUp();
    error_type_t result = adc_reader_init(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = adc_reader_deinit(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Check if the ADC reader can be re-initialized
    result = adc_reader_init(adc_reader);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    adcReaderTearDown();
}