#include <ads1115.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ADS1115";
static uint16_t READY_MODE_MAX_VOLTAGE_MV = 6144; // Maximum voltage in millivolts for READY mode with PGA set to 6.144V
static uint16_t READY_MODE_MIN_VOLTAGE_MV = -6144; // Minimum voltage in millivolts for READY mode with PGA set to 6.144V
static const float VOLTAGE_GAINS[] = {
    6.144,
    4.096,  
    2.048, 
    1.024,
    0.512,
    0.256
};


static const uint64_t FULL_RANGE = 1 << 15; // Full range for 16-bit signed integer

struct ads1115_t {
    ads1115_config_t config; // Configuration for the ADS1115
    bool is_initialized; // Flag to check if the ADS1115 is initialized
};

static error_type_t convert_register_value_to_voltage(const ads1115_t *ads, int16_t raw_value, int16_t *voltage_raw)
{
    if (ads == NULL || voltage_raw == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or voltage pointer
    }
    int16_t temp = raw_value;
    double voltage = (double)(temp) / FULL_RANGE; // Convert to voltage
    // Apply the PGA gain to the raw value
    voltage = (voltage * VOLTAGE_GAINS[ads->config.pga_mode]); // Get the voltage based on the PGA setting
    temp = (int16_t)(voltage * 1000);                           // Convert voltage to millivolts and store in raw_value
    *voltage_raw = temp;                                        //
    return SYSTEM_OK;                                           // Successfully converted raw value to voltage
}

static error_type_t read_register(const ads1115_t *ads, ads1115_register_address_t reg, uint8_t *buffer, size_t buffer_size)
{
    if (ads == NULL || buffer == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (buffer_size < 2)
    {
        return SYSTEM_INVALID_LENGTH; // Buffer size is insufficient
    }
    esp_err_t err = i2c_master_transmit_receive(ads->config.i2c_dev_handle,
                                                (uint8_t *)&reg, 1,
                                                buffer, buffer_size, 1000);
    if (err != ESP_OK)
    {
        return SYSTEM_OPERATION_FAILED;
    }
    return SYSTEM_OK; // Successfully read the register
}

static error_type_t write_register(const ads1115_t *ads, ads1115_register_address_t reg, const uint8_t *buffer, size_t buffer_size)
{
    if (ads == NULL || buffer == NULL)
    {
        return SYSTEM_NULL_PARAMETER;
    }
    if (buffer_size < 2)
    {
        return SYSTEM_INVALID_LENGTH; // Buffer size is insufficient
    }
    uint8_t *write_buffer = (uint8_t *)malloc(buffer_size + 1);
    if (write_buffer == NULL)
    {
        return SYSTEM_OPERATION_FAILED; // Memory allocation failed
    }
    write_buffer[0] = (uint8_t)reg;                // First byte is the register address
    memcpy(&write_buffer[1], buffer, buffer_size); // Copy the data to be written

    esp_err_t err = i2c_master_transmit(ads->config.i2c_dev_handle,
                                        write_buffer, buffer_size + 1, 1000);
    free(write_buffer); // Free the allocated memory
    if (err != ESP_OK)
    {
        return SYSTEM_OPERATION_FAILED;
    }
    return SYSTEM_OK; // Successfully wrote to the register
}

static error_type_t reset_internal_registers_and_power_down(ads1115_t *ads)
{
    if (ads == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    uint8_t reset_cmd = 0x06; // Reset command for ADS1115
    uint8_t first_message = 0;
    i2c_master_transmit_multi_buffer_info_t multi_buffer_info[2] = {
        {
            .write_buffer = &first_message,
            .buffer_size = 1 // Size of the first message
        },
        {
            .write_buffer = &reset_cmd, // Write the reset command
            .buffer_size = 1            // Size of the reset command
        }};

    error_type_t err = i2c_master_transmit(ads->config.i2c_dev_handle, (uint8_t *)multi_buffer_info, 2, 1000);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C transmit failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED; // Handle I2C transmit failure
    }
    return SYSTEM_OK; // Successfully reset the internal registers
}

static IRAM_ATTR void gpio_isr_handler(void *arg)
{
    ads1115_t *ads = (ads1115_t *)arg;
    // // Further processing can be done here, such as notifying a task or handling the value
    // // If a callback is registered, invoke it
    if(ads == NULL || !ads->is_initialized){
        return; // Handle null ADS1115, uninitialized state
    }
    if(ads->config.measurement_mode != ADS1115_MEASUREMENT_CONTINUOUS_OVERCURRENT && ads->config.measurement_mode != ADS1115_MEASUREMENT_CONTINUOUS_READY){
        return; // Not in a mode that requires handling the interrupt
    }
    if(ads->config.measurement_callback == NULL && ads->config.comparator_callback == NULL && ads->config.callback_context_ == NULL){
        return; // No callback registered, nothing to do
    }
    if(ads->config.measurement_mode == ADS1115_MEASUREMENT_CONTINUOUS_OVERCURRENT){
        overcurrent_queue_item_t item;
        item.context = (void*)arg;
        item.timestamp = xTaskGetTickCountFromISR();
        item.channel = ads->config.input_channel;
        item.callers_context = ads->config.callback_context_;
        ads->config.comparator_callback(item);
        return;
    } 
    if(ads->config.measurement_mode == ADS1115_MEASUREMENT_CONTINUOUS_READY){
        measurement_item_t item;
        item.context = (void*)arg;
        item.channel = ads->config.input_channel;
        ads->config.measurement_callback(item);
        return;
    }
}

ads1115_t *ads1115_create(const ads1115_config_t *config)
{
    ads1115_t *ads = (ads1115_t *)malloc(sizeof(ads1115_t));
    if (ads == NULL) {
        return NULL; // Handle memory allocation failure
    }
    // Copy the configuration
    memcpy(&ads->config, config, sizeof(ads1115_config_t));
    ads->is_initialized = false; // Initially not initialized

    return ads;
}

static ads1115_input_channel_t number_to_channel(uint8_t channel_number){
    switch(channel_number){
        case 0:
            return ADS1115_CHANNEL_0;
        case 1:
            return ADS1115_CHANNEL_1;
        case 2:
            return ADS1115_CHANNEL_2;
        case 3:
            return ADS1115_CHANNEL_3;
        default:
            return ADS1115_CHANNEL_0; // Default to channel 0 if invalid number is provided
    }
}

error_type_t ads1115_init(ads1115_t* ads){
    if(ads == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    if(ads->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADS1115 is already initialized
    }
    esp_err_t err;
    ads->config.input_channel = number_to_channel(ads->config.input_channel); // Convert the channel number to the corresponding enum value
    // set up interrupt for ALERT/READY pin if used
    if(ads->config.measurement_mode == ADS1115_MEASUREMENT_CONTINUOUS_OVERCURRENT || ads->config.measurement_mode == ADS1115_MEASUREMENT_CONTINUOUS_READY){
        err = gpio_isr_handler_add(ads->config.alert_ready_pin, gpio_isr_handler, (void *)ads);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add GPIO ISR handler: %s", esp_err_to_name(err));
            return SYSTEM_OPERATION_FAILED; // Handle failure to add GPIO ISR handler
        }
    }
    ads->is_initialized = true; // Set the initialized flag to true
    ESP_LOGI(TAG, "ADS1115 initialized successfully");
    return SYSTEM_OK; // Successfully initialized the ADS1115
}

error_type_t ads1115_read_one_shot(const ads1115_t *ads, int16_t *raw_value)
{
    if (ads == NULL || raw_value == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or raw value pointer
    }
    if (!ads->is_initialized)
    {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    if(ads->config.measurement_mode != ADS1115_MEASUREMENT_ONE_SHOT){
        return SYSTEM_INVALID_MODE; // ADS1115 is not in one-shot mode
    }
    // check if conversion is in progress
    ads1115_conversion_status_t status;
    error_type_t err;

    uint16_t config_reg_value = 0;
    config_reg_value |= 1 << 15;
    config_reg_value |= (ads->config.input_channel & 0x07) << 12;                   // Set the MUX bits
    // print channel for debugging
    ESP_LOGI(TAG, "Configuring ADS1115 for one-shot read on channel: %d", ads->config.input_channel);
    config_reg_value |= (ads->config.pga_mode & 0x07) << 9;            // Set the PGA bits
    config_reg_value |= (ADS1115_MODE_SINGLE_SHOT & 0x01) << 8;         // Set the mode bit
    config_reg_value |= (ADS1115_RATE_860_SPS & 0x07) << 5;             // Set the data rate bits
    config_reg_value |= (ADS1115_COMP_MODE_TRADITIONAL & 0x01) << 4;    // Set the comparator mode bit
    config_reg_value |= (ADS1115_COMP_POLARITY_ACTIVE_LOW & 0x01) << 3; // Set the comparator polarity bit
    config_reg_value |= (ADS1115_COMP_LATCHING_DISABLED & 0x01) << 2;   // Set the comparator latching bit
    config_reg_value |= (ADS1115_COMP_QUEUE_DISABLE & 0x03);            // Set the comparator queue bits

    uint8_t buffer[2];
    buffer[0] = (uint8_t)(config_reg_value >> 8);   // High byte of config register
    buffer[1] = (uint8_t)(config_reg_value & 0xFF); // Low byte of config register

    err = write_register(ads, ADS1115_CONFIG_REGISTER, buffer, 2);
    if (err != SYSTEM_OK)
    {
        return err;
    }

    err = read_register(ads, ADS1115_CONVERSION_REGISTER, buffer, 2);
    if (err != SYSTEM_OK)
    {
        return err;
    }

    // convert the received buffer to a 16-bit value
    // buffer[0] is the high byte and buffer[1] is the low byte
    int16_t value = (buffer[0] << 8) | buffer[1]; // Combine high and low bytes into a 16-bit value
    err = convert_register_value_to_voltage(ads, value, raw_value); // Convert to voltage for logging
    if (err != SYSTEM_OK)
    {
        return err;
    }
    // print the raw value and the voltage in millivolts for debugging
    ESP_LOGI(TAG, "One-shot read: raw value = %d, voltage = %d mV", value, *raw_value);
    return SYSTEM_OK; // Successfully read the value
}

error_type_t get_threshold_buffer(const ads1115_t *ads, const uint16_t threshold_value, uint8_t *buffer, size_t buffer_size)
{
    if (ads == NULL || buffer == NULL)
    {
        return SYSTEM_INVALID_PARAMETER; // Handle null ADS1115 or buffer pointer or insufficient buffer size
    }
    if (buffer_size < 2)
    {
        return SYSTEM_INVALID_LENGTH; // Buffer size is insufficient
    }
    double temp = threshold_value / (VOLTAGE_GAINS[ads->config.pga_mode] * 1000.0);
    uint16_t threshold_raw = (uint16_t)(temp * FULL_RANGE);
    buffer[0] = (uint8_t)(threshold_raw >> 8);   // High byte of lo threshold register
    buffer[1] = (uint8_t)(threshold_raw & 0xFF); // Low byte of lo threshold register
    return SYSTEM_OK;                            // Successfully prepared the threshold buffer
}

error_type_t ads1115_read_comparator(ads1115_t *ads, const uint16_t high_threshold_value_in_millivolt, const uint16_t low_threshold_value_in_millivolt)
{
    if (ads == NULL || ads->config.comparator_callback == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameter: ads=%p, comparator_callback=%p, context=%p", ads, ads->config.comparator_callback, ads->config.callback_context_);
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or raw value pointer
    }
    if (!ads->is_initialized)
    {
        ESP_LOGE(TAG, "ADS1115 is not initialized");
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    if(ads->config.measurement_mode != ADS1115_MEASUREMENT_CONTINUOUS_OVERCURRENT){
        ESP_LOGE(TAG, "ADS1115 is not in continuous overcurrent comparator mode");
        return SYSTEM_INVALID_MODE; // ADS1115 is not in comparator mode
    }
    error_type_t err_;
    // set the threshold registers
    uint8_t buffer[2];
    err_ = get_threshold_buffer(ads, low_threshold_value_in_millivolt, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to get threshold buffer for low threshold: %d", err_);
        return err_;
    }
    err_ = write_register(ads, ADS1115_LO_THRESH_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to write low threshold register: %d", err_);
        return err_;
    }

    err_ = get_threshold_buffer(ads, high_threshold_value_in_millivolt, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to get threshold buffer for high threshold: %d", err_);
        return err_;
    }
    err_ = write_register(ads, ADS1115_HI_THRESH_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to write high threshold register: %d", err_);
        return err_;
    }
    uint16_t config_reg_value = 0;
    config_reg_value |= (ads->config.input_channel & 0x07) << 12;                   // Set the MUX bits
    config_reg_value |= (ads->config.pga_mode & 0x07) << 9;            // Set the PGA bits
    config_reg_value |= (ADS1115_MODE_CONTINUOUS & 0x01) << 8;          // Set the mode bit
    config_reg_value |= (ADS1115_RATE_860_SPS & 0x07) << 5;             // Set the data rate bits
    config_reg_value |= (ADS1115_COMP_MODE_WINDOW & 0x01) << 4;         // Set the comparator mode bit
    config_reg_value |= (ADS1115_COMP_POLARITY_ACTIVE_LOW & 0x01) << 3; // Set the comparator polarity bit
    config_reg_value |= (ADS1115_COMP_LATCHING_DISABLED & 0x01) << 2;   // Set the comparator latching bit
    config_reg_value |= (ADS1115_COMP_QUEUE_1_CONVERSION & 0x03);       // Set the comparator queue bits

    buffer[0] = (uint8_t)(config_reg_value >> 8);   // High byte of config register
    buffer[1] = (uint8_t)(config_reg_value & 0xFF); // Low byte of config register

    err_ = write_register(ads, ADS1115_CONFIG_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        ESP_LOGE(TAG, "Failed to write config register: %d", err_);
        return err_;
    }
    return SYSTEM_OK;
}


error_type_t ads1115_read_continuous(ads1115_t* ads){
    if (ads == NULL || ads->config.measurement_callback == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or raw value pointer
    }
    if (!ads->is_initialized || ads->config.measurement_mode != ADS1115_MEASUREMENT_CONTINUOUS_READY)
    {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    error_type_t err_;
    // set the threshold registers
    uint8_t buffer[2];
    // the READY mode uses max measurable voltage as threshold so that the comparator only trigger in worse case max voltage
    err_ = get_threshold_buffer(ads, READY_MODE_MIN_VOLTAGE_MV, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        return err_;
    }
    // clear the MSB to set to READY mode
    buffer[0] &= 0x7F;
    err_ = write_register(ads, ADS1115_LO_THRESH_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        return err_;
    }
    // the READY mode uses min measurable voltage as threshold so that the comparator only trigger in worse case min voltage
    err_ = get_threshold_buffer(ads, READY_MODE_MAX_VOLTAGE_MV, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        return err_;
    }
    // set the MSB to set to READY mode
    buffer[0] |= 0x80;
    err_ = write_register(ads, ADS1115_HI_THRESH_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        return err_;
    }
    uint16_t config_reg_value = 0;
    config_reg_value |= (ads->config.input_channel & 0x07) << 12;                   // Set the MUX bits
    config_reg_value |= (ads->config.pga_mode & 0x07) << 9;            // Set the PGA bits
    config_reg_value |= (ADS1115_MODE_CONTINUOUS & 0x01) << 8;          // Set the mode bit
    config_reg_value |= (ADS1115_RATE_475_SPS & 0x07) << 5;             // Set the data rate bits
    config_reg_value |= (ADS1115_COMP_MODE_TRADITIONAL & 0x01) << 4;         // Set the comparator mode bit
    config_reg_value |= (ADS1115_COMP_POLARITY_ACTIVE_LOW & 0x01) << 3; // Set the comparator polarity bit
    config_reg_value |= (ADS1115_COMP_LATCHING_DISABLED & 0x01) << 2;   // Set the comparator latching bit
    config_reg_value |= (ADS1115_COMP_QUEUE_1_CONVERSION & 0x03);       // Set the comparator queue bits

    buffer[0] = (uint8_t)(config_reg_value >> 8);   // High byte of config register
    buffer[1] = (uint8_t)(config_reg_value & 0xFF); // Low byte of config register

    err_ = write_register(ads, ADS1115_CONFIG_REGISTER, buffer, sizeof(buffer));
    if (err_ != SYSTEM_OK)
    {
        return err_;
    }
    return SYSTEM_OK;
}

error_type_t ads1115_read_conversion_register(ads1115_t* ads, ads1115_input_channel_t input_channel, int16_t* raw_value){
    if( ads == NULL || raw_value == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or raw value pointer
    }
    if (!ads->is_initialized)
    {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    // read the conversion register
    uint8_t buffer[2];
    error_type_t err = read_register(ads, ADS1115_CONVERSION_REGISTER, buffer, sizeof(buffer));
    if (err != SYSTEM_OK)
    {
        return err;
    }
    // convert the received buffer to a 16-bit value
    // buffer[0] is the high byte and buffer[1] is the low byte
    int16_t value = (buffer[0] << 8) | buffer[1]; // Combine high and low bytes into a 16-bit value
    err = convert_register_value_to_voltage(ads, value, raw_value); // Convert to voltage for logging
    if (err != SYSTEM_OK)
    {
        return err;
    }
    return SYSTEM_OK; // Successfully read the value
}

error_type_t ads1115_deinit(ads1115_t *ads)
{
    if (ads == NULL)
    {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    if (!ads->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    ads->is_initialized = false; // Reset the initialized flag
    ESP_LOGI(TAG, "ADS1115 deinitialized successfully");
    return SYSTEM_OK;
}
error_type_t ads1115_destroy(ads1115_t** ads){
    if (ads == NULL || *ads == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 pointer
    }

    if ((*ads)->is_initialized) {
        error_type_t err = ads1115_deinit(*ads);
        if (err != SYSTEM_OK) {
            ESP_LOGE(TAG, "Deinitialization failed: %d", err);
            return err; // Handle deinitialization failure
        }
    }

    free(*ads); // Free the ADS1115 structure
    *ads = NULL; // Set pointer to NULL after freeing
    ESP_LOGI(TAG, "ADS1115 destroyed successfully");
    return SYSTEM_OK;
}