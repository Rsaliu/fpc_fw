#include "current_analytics.h"
#include <math.h>

error_type_t current_analytics_basic_decision(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state){
    if(sampled_current_values == NULL || state == NULL || number_of_samples <= 0){
        return SYSTEM_NULL_PARAMETER;
    }
    // Calculate the average current from the sampled values
    float sum = 0.0f;
    for(int i = 0; i < number_of_samples; ++i){
        sum += abs(sampled_current_values[i]);
    }
    float average_current = sum / number_of_samples;

    // Decision logic based on the average current
    if(average_current >= rated_current){
        *state = PUMP_STATE_MACHINE_OVERCURRENT_STATE;
        ESP_LOGW("CURRENT_ANALYTICS", "Pump is in overcurrent state! Average current: %.2f A", average_current);
    } else if (average_current < rated_current && average_current >= min_working_current) {
        *state = PUMP_STATE_MACHINE_NORMAL_STATE;
        ESP_LOGI("CURRENT_ANALYTICS", "Pump is in normal state. Average current: %.2f A", average_current);
    } else {
        *state = PUMP_STATE_MACHINE_UNDERCURRENT_STATE;
        ESP_LOGW("CURRENT_ANALYTICS", "Pump is in undercurrent state! Average current: %.2f A", average_current);
    }
    return SYSTEM_OK;
}