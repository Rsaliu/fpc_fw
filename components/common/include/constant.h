#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "driver/adc.h"
#include "ads1115.h"
#include "driver/gpio.h"


typedef enum {
    CHANNEL_0_CONST = 0,
    CHANNEL_1_CONST,
    CHANNEL_2_CONST,
    CHANNEL_3_CONST
} channel_t;

static const adc_reader_config_t DEFAULT_ADC_CONFIG = {
    .adc_unit_id = ADC_UNIT_1,
    .adc_channel = ADC_CHANNEL_0, 
    .adc_atten = ADC_ATTEN_DB_11,
    .adc_bitwidth = ADC_BITWIDTH_DEFAULT
};

static const ads1115_config_t DEFAULT_I2C_CONFIG = {
    .i2c_port = -1,
    .sda_gpio = GPIO_NUM_21,
    .scl_gpio = GPIO_NUM_22,
    .clock_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .enable_pullup = true,
    .frequency_hz = 400000,
    .i2c_address = 0x48,
    .pga_mode = ADS1115_PGA_2_048V
};

#endif // __CONSTANT_H__


