#ifndef __ADC_READER_H__
#define __ADC_READER_H__

#include <common_headers.h>
#include <esp_adc/adc_oneshot.h>

typedef struct {
    adc_unit_t adc_unit_id; // ADC unit ID
    adc_channel_t adc_channel; // ADC channel
    adc_atten_t adc_atten; // ADC attenuation
    adc_bitwidth_t adc_bitwidth; // ADC bit width
} adc_reader_config_t;
typedef struct adc_reader_t adc_reader_t;


adc_reader_t* adc_reader_create(const adc_reader_config_t* config);
error_type_t adc_reader_init(adc_reader_t* adc_reader);
error_type_t adc_reader_read(const adc_reader_t* adc_reader, int *raw_value);
error_type_t adc_reader_deinit(adc_reader_t* adc_reader);
error_type_t adc_reader_destroy(adc_reader_t** adc_reader);
#endif // __ADC_READER_H__