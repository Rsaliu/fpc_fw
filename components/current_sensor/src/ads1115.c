#include <ads1115.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ADS1115";
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
    ads1115_config_t* config; // Configuration for the ADS1115
    bool is_initialized; // Flag to check if the ADS1115 is initialized
    i2c_master_bus_handle_t i2c_handle; // I2C handle for communication
     i2c_master_dev_handle_t i2c_dev_handle; // I2C device handle
};




ads1115_t* ads1115_create(const ads1115_config_t* config){
    ads1115_t *ads = (ads1115_t *)malloc(sizeof(ads1115_t));
    if (ads == NULL) {
        return NULL; // Handle memory allocation failure
    }
    ads->config = config; // Copy the configuration
    ads->is_initialized = false; // Initially not initialized

    return ads;
}
error_type_t ads1115_init(ads1115_t* ads){
    if(ads == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    if(ads->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADS1115 is already initialized
    }
       i2c_master_bus_config_t bus_config = {
        .i2c_port = ads->config->i2c_port,
        .sda_io_num = ads->config->sda_gpio,
        .scl_io_num = ads->config->scl_gpio,
        .clk_source = ads->config->clock_source,
        .glitch_ignore_cnt = ads->config->glitch_ignore_cnt,
        .flags.enable_internal_pullup = ads->config->enable_pullup,
    }; 
    esp_err_t err;
     err = i2c_new_master_bus(&bus_config, &ads->i2c_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus creation failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED; // Handle I2C bus creation failure
    }
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ads->config->i2c_address,
        .scl_speed_hz = ads->config->frequency_hz,
    };
    err = i2c_master_bus_add_device(ads->i2c_handle, &dev_config, &ads->i2c_dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C device creation failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED; // Handle I2C device creation failure
    }
    ads->is_initialized = true; // Set the initialized flag to true
    ESP_LOGI(TAG, "ADS1115 initialized successfully");
    return SYSTEM_OK; // Successfully initialized the ADS1115
}


static error_type_t reset_internal_registers_and_power_down(ads1115_t* ads){
    if (ads == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    uint8_t reset_cmd = 0x06; // Reset command for ADS1115
    uint8_t first_message = 0;
    i2c_master_transmit_multi_buffer_info_t multi_buffer_info[2] = {
        {.write_buffer = &first_message, 
        .buffer_size = 1 // Size of the first message
        },
        {.write_buffer = &reset_cmd, // Write the reset command 
        .buffer_size = 1 // Size of the reset command
        }
    };

    error_type_t err = i2c_master_transmit(ads->i2c_dev_handle, (uint8_t*)multi_buffer_info,2, 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C transmit failed: %s", esp_err_to_name(err));
        return SYSTEM_OPERATION_FAILED; // Handle I2C transmit failure
    }
    return SYSTEM_OK; // Successfully reset the internal registers
}

error_type_t ads1115_read(const ads1115_t* ads, int16_t* raw_value, ads1115_input_channel_t input_channel){
    if (ads == NULL || raw_value == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115 or raw value pointer
    }
    if (!ads->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }
    ads1115_register_address_t reg = ADS1115_CONFIG_REGISTER;

    uint16_t config_reg_value = 0;
    config_reg_value |= 1 << 15;
    config_reg_value |= (input_channel & 0x07) << 12; // Set the MUX bits
    config_reg_value |= (ads->config->pga_mode & 0x07) << 9; // Set the PGA bits
    config_reg_value |= (ADS1115_MODE_SINGLE_SHOT & 0x01) << 8; // Set the mode bit
    config_reg_value |= (ADS1115_RATE_128_SPS & 0x07) << 5; // Set the data rate bits
    config_reg_value |= (ADS1115_COMP_MODE_TRADITIONAL & 0x01) << 4; // Set the comparator mode bit
    config_reg_value |= (ADS1115_COMP_POLARITY_ACTIVE_LOW & 0x01) << 3; // Set the comparator polarity bit
    config_reg_value |= (ADS1115_COMP_LATCHING_DISABLED & 0x01) << 2; // Set the comparator latching bit
    config_reg_value |= (ADS1115_COMP_QUEUE_DISABLE & 0x03); // Set the comparator queue bits

    uint8_t buffer[3];
    buffer[0] = (uint8_t)reg;
    buffer[1] = (uint8_t)(config_reg_value >> 8); // High byte of config register
    buffer[2] = (uint8_t)(config_reg_value & 0xFF); // Low byte of config register
    // Ensure the buffer is properly set up for transmission
    //print each byte in config_reg



    //print buffer
    ESP_LOGI(TAG, "ADS1115 write buffer: 0x%02X 0x%02X 0x%02X", buffer[0], buffer[1], buffer[2]);



    esp_err_t err = i2c_master_transmit(ads->i2c_dev_handle, buffer, 3, 1000);
    if(err != ESP_OK){
        return SYSTEM_OPERATION_FAILED;
    }
    ESP_LOGI("ADS1115", "ADS1115 write first 3 bytes successful");
    reg = ADS1115_CONVERSION_REGISTER;

    err = i2c_master_transmit_receive(ads->i2c_dev_handle,
                                        (uint8_t*)&reg, 1,
                                         buffer, 2, 1000);
    if(err != ESP_OK){
        return SYSTEM_OPERATION_FAILED;
    }
    //print received buffer
    ESP_LOGI(TAG, "ADS1115 read buffer: 0x%02X 0x%02X", buffer[0], buffer[1]);

    //convert the received buffer to a 16-bit value
    //buffer[0] is the high byte and buffer[1] is the low byte
    int16_t value = (buffer[0] << 8) | buffer[1]; // Combine high and low bytes into a 16-bit value
    double voltage = (double)(value) / FULL_RANGE; // Convert to voltage
    // Apply the PGA gain to the raw value
    voltage = (voltage * VOLTAGE_GAINS[ads->config->pga_mode]); // Get the voltage based on the PGA setting
    value = (int16_t)(voltage * 1000); // Convert voltage to millivolts and store in raw_value
    *raw_value = value; // Store the raw value in the provided pointer
    ESP_LOGI(TAG, "ADS1115 read value: %d", *raw_value);
    return SYSTEM_OK; // Successfully read the value
}

error_type_t ads1115_deinit(ads1115_t* ads){
    if (ads == NULL) {
        return SYSTEM_NULL_PARAMETER; // Handle null ADS1115
    }
    if (!ads->is_initialized) {
        return SYSTEM_INVALID_STATE; // ADS1115 is not initialized
    }

    // esp_err_t err = reset_internal_registers_and_power_down(ads);
    // if (err != SYSTEM_OK) {
    //     ESP_LOGE(TAG, "Failed to reset internal registers: %d", err);
    //     return err; // Handle reset failure
    // }

    if (ads->i2c_dev_handle != NULL) {
        i2c_master_bus_rm_device(ads->i2c_dev_handle);
        ads->i2c_dev_handle = NULL; // Reset the I2C device handle
    }

    if( ads->i2c_handle != NULL) {
        i2c_del_master_bus(ads->i2c_handle);
        ads->i2c_handle = NULL; // Reset the I2C handle
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