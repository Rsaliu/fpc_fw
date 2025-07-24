#ifndef __ADS1115_H__
#define __ADS1115_H__
#include "driver/i2c_master.h"

#include "common_headers.h"
typedef enum{
    ADS1115_CONVERSION_REGISTER,
    ADS1115_CONFIG_REGISTER,
    ADS1115_LO_THRESH_REGISTER,
    ADS1115_HI_THRESH_REGISTER
}ads1115_register_address_t;

typedef enum{
    ADS1115_CHANNEL_DIFF_0_1 = 0x00, // Differential channel between 0 and 1
    ADS1115_CHANNEL_DIFF_0_3 = 0x01, // Differential channel between 0 and 3
    ADS1115_CHANNEL_DIFF_1_3 = 0x02, // Differential channel between 1 and 3
    ADS1115_CHANNEL_DIFF_2_3 = 0x03, // Differential channel between 2 and 3
    ADS1115_CHANNEL_0 = 0x04, // Channel 0
    ADS1115_CHANNEL_1 = 0x05, // Channel 1
    ADS1115_CHANNEL_2 = 0x06, // Channel 2
    ADS1115_CHANNEL_3 = 0x07, // Channel 3
}ads1115_input_channel_t;

typedef enum{
    ADS1115_PGA_6_144V = 0x00, // Programmable Gain Amplifier for 6.144V
    ADS1115_PGA_4_096V = 0x01, // Programmable Gain Amplifier for 4.096V
    ADS1115_PGA_2_048V = 0x02, // Programmable Gain Amplifier for 2.048V
    ADS1115_PGA_1_024V = 0x03, // Programmable Gain Amplifier for 1.024V
    ADS1115_PGA_0_512V = 0x04, // Programmable Gain Amplifier for 0.512V
    ADS1115_PGA_0_256V = 0x05, // Programmable Gain Amplifier for 0.256V
}ads1115_pga_mode_t;



typedef enum{
    ADS1115_MODE_CONTINUOUS = 0x00, // Continuous conversion mode
    ADS1115_MODE_SINGLE_SHOT = 0x01, // Single-shot conversion mode
}ads1115_mode_t;

typedef enum{
    ADS1115_RATE_8_SPS = 0x00, // 8 samples per second
    ADS1115_RATE_16_SPS = 0x01, // 16 samples per second
    ADS1115_RATE_32_SPS = 0x02, // 32 samples per second
    ADS1115_RATE_64_SPS = 0x03, // 64 samples per second
    ADS1115_RATE_128_SPS = 0x04, // 128 samples per second
    ADS1115_RATE_250_SPS = 0x05, // 250 samples per second
    ADS1115_RATE_475_SPS = 0x06, // 475 samples per second
    ADS1115_RATE_860_SPS = 0x07, // 860 samples per second      
}ads1115_data_rate_t;

typedef enum{
    ADS1115_COMP_MODE_TRADITIONAL = 0x00, // Traditional comparator mode
    ADS1115_COMP_MODE_WINDOW = 0x01, // Window comparator mode
}ads1115_comp_mode_t;

typedef enum{
    ADS1115_COMP_POLARITY_ACTIVE_LOW = 0x00, // Active low polarity
    ADS1115_COMP_POLARITY_ACTIVE_HIGH = 0x01, // Active high polarity   
}ads1115_comp_polarity_t;

typedef enum{
    ADS1115_COMP_LATCHING_DISABLED = 0x00, // Latching comparator disabled
    ADS1115_COMP_LATCHING_ENABLED = 0x01, // Latching comparator enabled
}ads1115_comp_latching_t;

typedef enum{
    ADS1115_COMP_QUEUE_DISABLE = 0x00, // Disable comparator queue
    ADS1115_COMP_QUEUE_1_CONVERSION = 0x01, // 1 conversion before alert
    ADS1115_COMP_QUEUE_2_CONVERSIONS = 0x02, // 2 conversions before alert
    ADS1115_COMP_QUEUE_4_CONVERSIONS = 0x03, // 4 conversions before alert
}ads1115_comp_queue_t;


typedef enum{
    ADDR_GROUNDED = 0x48, // Grounded address
    ADDR_VCC = 0x49, // VCC address
    ADD_SCL = 0x4A, // SCL address
    ADD_SDA = 0x4B, // SDA address
}ads1115_addr_t;

typedef struct {
    i2c_port_t i2c_port; // I2C port number
    gpio_num_t sda_gpio; // SDA GPIO pin
    gpio_num_t scl_gpio; // SCL GPIO pin
    uint32_t frequency_hz; // I2C frequency
    i2c_clock_source_t clock_source; // Clock source for the I2C bus
    uint8_t glitch_ignore_cnt; // Number of glitches to ignore
    uint32_t enable_pullup; // Enable pull-up resistors
    ads1115_addr_t i2c_address; // Address connection type
    ads1115_pga_mode_t pga_mode; // Programmable Gain Amplifier mode
} ads1115_config_t;

typedef struct ads1115_t ads1115_t;

ads1115_t* ads1115_create(const ads1115_config_t* config);
error_type_t ads1115_init(ads1115_t* ads);
error_type_t ads1115_read(const ads1115_t* ads, int16_t* raw_value, ads1115_input_channel_t input_channel);
error_type_t ads1115_deinit(ads1115_t* ads);
error_type_t ads1115_destroy(ads1115_t** ads);

#endif // __ADS1115_H__