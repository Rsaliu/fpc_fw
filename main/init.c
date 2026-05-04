#include "init.h"
#define ESP_INTR_FLAG_DEFAULT 0
#define MAIN_LOAD_FACTOR 0.75 // Load factor for the hash map
#define MAX_MAP_ITEMS 10
#define PUMP_MONITOR_NUMBER_OF_SAMPLES_FOR_AVERAGE 5


static const long SCL_SPEED_HZ_FOR_TESTING = 100000; // I2C clock speed for testing

static const i2c_master_bus_config_t i2c_bus_config_one = {
    .i2c_port = I2C_NUM_1,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 0,
    .flags.enable_internal_pullup = 1
};

static const i2c_device_config_t i2c_dev_config_one = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = ADDR_GROUNDED,
    .scl_speed_hz = SCL_SPEED_HZ_FOR_TESTING,
};

static const gpio_num_t ALERT_READY_PIN_OPTION_1 = GPIO_NUM_26; // GPIO pin for ALERT/READY signal during testing

// define i2c device handle for testing
static i2c_master_dev_handle_t i2c_dev_handle_option_1=NULL;
static i2c_master_bus_handle_t i2c_handle_option_1=NULL;

static const gpio_num_t RS485_ONE_DE_PIN = GPIO_NUM_5; // DE/RE pin for RS485 transceiver
static const gpio_num_t RS485_ONE_DI_PIN = GPIO_NUM_17; // Driver Input pin (connected to TX)
static const gpio_num_t RS485_ONE_RO_PIN = GPIO_NUM_16; // Receiver Output pin (connected to RX)
static const int RS485_ONE_BAUD_RATE = 9600; // Baud rate for RS485 communication
static const rs485_config_t rs485_config_one = {
     .uart_num = 1,
     .rs485_di_pin = RS485_ONE_DI_PIN, //driver input goes to tx pin 
     .rs485_ro_pin = RS485_ONE_RO_PIN, // receiver output goes to rx pin
     .rs485_dir_pin = RS485_ONE_DE_PIN, // de/re for bidirectional communication.
     .baud_rate = RS485_ONE_BAUD_RATE
};

static rs485_t* rs485_one = NULL;


static error_type_t dummy_current_analytics_callback(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state){
    ESP_LOGI("DUMMY_CURRENT_ANALYTICS_CALLBACK", "Current analytics callback triggered with %d samples, rated current: %.2f A, min working current: %.2f A", number_of_samples, rated_current, min_working_current);
    // generate random state for testing
    int random_value = esp_random() % 3; // Generate a random number between 0 and 2
    ESP_LOGI("DUMMY_CURRENT_ANALYTICS_CALLBACK", "Generated random state value: %d", random_value);
    *state = (pump_state_machine_state_t)random_value; // Cast the random number
    return SYSTEM_OK;
}

static error_type_t twenty_percent_min_analytics_callback(float* sampled_current_values, int number_of_samples, float rated_current, float min_working_current, pump_state_machine_state_t* state){
    if(sampled_current_values == NULL || state == NULL){
        ESP_LOGE("ANALYTICS_CALLBACK", "Invalid parameter(s) provided to analytics callback");
        return SYSTEM_NULL_PARAMETER;
    }
    // check if 20% of the values are above the threshold of max rated current. if yes return ovecurrent, if peak value is less than min_working_current return undercurrent , else return normal
    int count_above_rated = 0;
    // we will neglect undercurrent situation for now, can be handled later.
    for(int i = 0; i < number_of_samples; ++i){
        if(abs(sampled_current_values[i]) > rated_current){
            count_above_rated++;
        }
    }
    if(count_above_rated >= 0.2 * number_of_samples){
        *state = PUMP_STATE_MACHINE_OVERCURRENT_STATE;
    }
    else{
        *state = PUMP_STATE_MACHINE_NORMAL_STATE;
    }
    // log the counts and the resulting state for debugging
    ESP_LOGW("ANALYTICS_CALLBACK", "Count above rated current: %d, Resulting state: %d", count_above_rated, *state);
    return SYSTEM_OK;
}

error_type_t dummy_level_analytics_callback(int* sampled_level_values, int number_of_samples, int full_level, int low_level, tank_state_machine_state_t* state){
    ESP_LOGI("DUMMY_LEVEL_ANALYTICS_CALLBACK", "Current analytics callback triggered with %d samples, full_level: %d A, low_level: %d ", number_of_samples, full_level, low_level);
    // generate random state for testing
    int random_value = esp_random() % 3; // Generate a random number between 0 and 2
    ESP_LOGI("DUMMY_LEVEL_ANALYTICS_CALLBACK", "Generated random state value: %d", random_value);
    *state = (tank_state_machine_state_t)random_value; // Cast the random number
    return SYSTEM_OK;
}

error_type_t level_sensor_average_analytics_callback(uint16_t* sampled_level_values, int number_of_samples, int full_level, int low_level, tank_state_machine_state_t* state){
    if(sampled_level_values == NULL || state == NULL){
        ESP_LOGE("LEVEL_ANALYTICS_CALLBACK", "Invalid parameter(s) provided to level analytics callback");
        return SYSTEM_NULL_PARAMETER;
    }
    int sum = 0;
    for(int i = 0; i < number_of_samples; ++i){
        // print each sampled level value for debugging
        ESP_LOGI("LEVEL_ANALYTICS_CALLBACK", "Sampled level value for sample %d: %d", i, sampled_level_values[i]);
        sum += sampled_level_values[i];
    }
    int average_level = sum / number_of_samples;
    if(average_level >= full_level){
        *state = TANK_STATE_MACHINE_FULL_STATE;
    }
    else if(average_level <= low_level){
        *state = TANK_STATE_MACHINE_LOW_STATE;
    }
    else{
        *state = TANK_STATE_MACHINE_NORMAL_STATE;
    }
    ESP_LOGI("LEVEL_ANALYTICS_CALLBACK", "Average level: %d, Resulting state: %d", average_level, *state);
    return SYSTEM_OK;
}

static void dummy_pump_monitor_event_callback(void* context, event_type_t state,int monitor_id){
    ESP_LOGI("DUMMY_CALLBACK", "Pump monitor event callback triggered for monitor ID: %d with state: %d", monitor_id, state);
}

static void dummy_pump_monitor_event_callback_two(void* context, event_type_t state,int monitor_id){
    ESP_LOGI("DUMMY_CALLBACK TWO", "Pump monitor event callback triggered for monitor ID: %d with state: %d", monitor_id, state);
}

static void dummy_relay_response_callback(void* context, event_type_t state,int monitor_id){
    ESP_LOGW("DUMMY_RELAY_RESPONSE_CALLBACK", "Relay response callback triggered for monitor ID: %d with state: %d and context: %p", monitor_id, state, context);
    if(context == NULL){
        ESP_LOGE("DUMMY_RELAY_RESPONSE_CALLBACK", "Context is NULL in dummy relay response callback for monitor ID: %d", monitor_id);
        return;
    }
    error_type_t err;
    if(state == EVENT_PUMP_OVERCURRENT){
         ESP_LOGW("DUMMY_RELAY_RESPONSE_CALLBACK", "Pump overcurrent event received in dummy relay response callback for monitor ID: %d, tripping relay", monitor_id);
        err = relay_trip((relay_t*)context);
        if(err != SYSTEM_OK){
            ESP_LOGE("DUMMY_RELAY_RESPONSE_CALLBACK", "Failed to trip relay in dummy relay response callback for monitor ID: %d with error code: %d", monitor_id, err);
            return;
        }    
    }
    else{
        ESP_LOGW("DUMMY_RELAY_RESPONSE_CALLBACK", "Pump normal event received in dummy relay response callback for monitor ID: %d, resetting relay", monitor_id);
        err = relay_on((relay_t*)context);
        if(err != SYSTEM_OK){
            ESP_LOGE("DUMMY_RELAY_RESPONSE_CALLBACK", "Failed to to put relay ON response callback for monitor ID: %d with error code: %d", monitor_id, err);
            return;
        }
          
    }

}


error_type_t initializa_hw_peripherals(){
    // This function can be used to setup any peripherals, GPIOs, I2C, SPI, etc. required for testing
    // For example, you can initialize the I2C bus here if your sensors require it

    error_type_t err;
    err = i2c_new_master_bus(&i2c_bus_config_one, &i2c_handle_option_1);
     if(err != SYSTEM_OK){
        ESP_LOGE("SETUP_PERIPHERALS", "Failed to initialize I2C master bus: %d", err);
        return SYSTEM_FAILED;
    }
    if(i2c_handle_option_1 == NULL){
        ESP_LOGE("SETUP_PERIPHERALS", "I2C master bus handle is NULL after initialization");
        return SYSTEM_FAILED;
    }

    err = i2c_master_bus_add_device(i2c_handle_option_1, &i2c_dev_config_one, &i2c_dev_handle_option_1);
    if(err != SYSTEM_OK){
        ESP_LOGE("SETUP_PERIPHERALS", "Failed to add I2C device to bus: %d", err);
        return SYSTEM_FAILED;
    }
    if(i2c_dev_handle_option_1 == NULL){
        ESP_LOGE("SETUP_PERIPHERALS", "I2C device handle is NULL after adding device to bus");
        return SYSTEM_FAILED;
    }
    // print pointer value of i2c_dev_handle_option_1 for debugging
    ESP_LOGI("SETUP_PERIPHERALS", "Pointer value of I2C device handle after adding device to bus: %p", (void*)i2c_dev_handle_option_1);
    err = globals_set_i2c_dev_handle_option(1, i2c_dev_handle_option_1);
    if(err != SYSTEM_OK){
        ESP_LOGE("SETUP_PERIPHERALS", "Failed to set I2C bus handle in globals: %d", err);
        return SYSTEM_FAILED;
    }
    // print pointer value of globals_get_i2c_bus_handle_option(1) for debugging
    ESP_LOGI("SETUP_PERIPHERALS", "Pointer value of I2C bus handle from globals: %p", (void*)globals_get_i2c_dev_handle_option(1));

    // setup gpio for ADS1115 ALERT/READY pin
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << ALERT_READY_PIN_OPTION_1),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    rs485_one = rs485_create(&rs485_config_one);
    if(rs485_one == NULL){
        ESP_LOGE("SETUP_PERIPHERALS", "Failed to create RS485 instance");
        return SYSTEM_FAILED;
    }
    err = rs485_init(rs485_one);
    if(err != SYSTEM_OK){
        ESP_LOGE("SETUP_PERIPHERALS", "Failed to initialize RS485 instance: %d", err);
        return SYSTEM_FAILED;
    }
    return SYSTEM_OK;
    
}

static pump_monitor_event_callback_t get_relay_response_callback(relay_response_type_t response_type){
    ESP_LOGI("GET_RELAY_RESPONSE_CALLBACK", "Getting relay response callback for response type: %d", response_type);
    switch(response_type){
        case RELAY_RESPONSE_ONE:
            return dummy_relay_response_callback;
        default:
            return NULL;
    }
}

static const char *valid_json =
"{\n"
"  \"site_id\": \"Site123\",\n"
"  \"device_id\": \"Device456\",\n"
"  \"pump_control_units\": [\n"
"    {\n"
"      \"id\": 1,\n"
"      \"tank_monitors\": [\n"
"        { \"id\": 1, \"tank_id\": 1, \"level_sensor_id\": 1 }\n"
"      ],\n"
"      \"pump_monitors\": [\n"
"        { \"id\": 1, \"pump_id\": 1, \"current_sensor_id\": 2 }\n"
"      ],\n"
"      \"tanks\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"capacity_litres\": 1000.0,\n"
"          \"shape\": \"RECTANGULAR\",\n"
"          \"height_cm\": 200.0,\n"
"          \"full_level_mm\": 1800,\n"
"          \"low_level_mm\": 200\n"
"        }\n"
"      ],\n"
"      \"pumps\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"make\": \"TestPump\",\n"
"          \"power_in_hp\": 2.5,\n"
"          \"current_rating\": 10.0,\n"
"          \"min_working_current\": 0.5\n"
"        }\n"
"      ],\n"
"      \"relays\": [\n"
"        { \"id\": 1, \"pin_number\": 23 }\n"
"      ],\n"
"      \"current_sensors\": [\n"
"        {\n"
"          \"id\": 1,\n"
"          \"interface\": {\n"
"            \"type\": \"ADS1115_one\",\n"
"            \"channel\": 1\n"
"          },\n"
"          \"make\": \"ACS712\",\n"
"          \"max_current\": 20,\n"
"          \"read_mode\": \"basic\"\n"
"        },\n"
"        {\n"
"          \"id\": 2,\n"
"          \"interface\": {\n"
"            \"type\": \"internal_adc\",\n"
"            \"channel\": 0\n"
"          },\n"
"          \"make\": \"ACS712\",\n"
"          \"max_current\": 20,\n"
"          \"read_mode\": \"basic\"\n"
"        }\n"
"      ],\n"
"      \"level_sensors\": [ \n"
"        {\n"
"        \"id\": 1,\n"
"        \"interface\": \"RS485\",\n"
"        \"address\": 1,\n"
"        \"protocol\": \"GA1\"\n"
"       }\n"
"       ],\n"
"      \"subscriptions\": [\n"
"        {\n"
"          \"monitor_type\": \"PUMP_MONITOR\",\n"
"          \"monitor_id\": 1,\n"
"          \"subscribers\": [\n"
"            {\n"
"              \"type\": \"RELAY\",\n"
"              \"id\": 1,\n"
"              \"response_type\": \"RELAY_RESPONSE_ONE\"\n"
"            }\n"
"          ]\n"
"        }\n"
"      ]\n"
"    }\n"
"  ]\n"
"}";

error_type_t initialize_system_from_json(const char* valid_json, size_t size, pump_control_unit_t** pump_control_unit_manager, size_t* num_pump_control_units){
    // This function can be used to initialize the system from a JSON configuration string, it can be called from main after setup_preripherals to initialize the system for testing
    // For example, you can parse the JSON and create pump control units, pumps, sensors, etc. based on the configuration
    HashMap pump_map;
    HashMap current_sensor_map;
    HashMap pump_monitor_map;
    HashMap tank_map;
    HashMap level_sensor_map;
    HashMap tank_monitor_map;
    HashMap relay_map;
    HashMap tank;
    error_type_t err;
    current_sensor_setup_config_t* current_sensor_configs = NULL;
    pump_setup_config_t* pump_configs = NULL;
    pump_monitor_setup_config_t* pump_monitor_configs = NULL;
    relay_setup_config_t* relay_configs = NULL;
    susbscription_setup_config_t* subscription_configs = NULL;
    tank_setup_config_t* tank_configs = NULL;
    level_sensor_setup_config_t* level_sensor_configs = NULL;
    tank_monitor_setup_config_t* tank_monitor_configs = NULL;
    cJSON* json_root = deserialized_to_json(valid_json, size);
    if(json_root == NULL){
        ESP_LOGE("APP_MAIN", "Failed to deserialize JSON string");
        return SYSTEM_FAILED;
    }
    ESP_LOGI("APP_MAIN", "Successfully deserialized JSON string");
    size_t num_pump_cus = 0;
    cJSON* pcus_json = get_pump_control_json_array_from_root(json_root, &num_pump_cus);
    if(pcus_json == NULL){
        ESP_LOGE("APP_MAIN", "Failed to get pump control units JSON array from root");
        cJSON_Delete(json_root);
        return SYSTEM_FAILED;
    }
    *num_pump_control_units = num_pump_cus; // Set the output parameter for number of pump control units
    if(num_pump_cus > MAX_PUMP_CONTROL_UNIT_COUNT){
        ESP_LOGE("APP_MAIN", "Number of pump control units in JSON exceeds maximum supported count of %d", MAX_PUMP_CONTROL_UNIT_COUNT);
        cJSON_Delete(json_root);
        return SYSTEM_FAILED;
    }
    emhashmap_initialize(&pump_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR); 
    emhashmap_initialize(&current_sensor_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&pump_monitor_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&tank_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&level_sensor_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&tank_monitor_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&relay_map,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    emhashmap_initialize(&tank,MAX_MAP_ITEMS,MAIN_LOAD_FACTOR);
    ESP_LOGI("APP_MAIN", "Successfully retrieved pump control units JSON array from root with %d pump control units", num_pump_cus);
    for(size_t i = 0; i < num_pump_cus; i++){
        pump_control_unit_t* pump_control_unit = pump_control_unit_create();
        pump_control_unit_manager[i] = pump_control_unit; // Store the pointer to the pump control unit instance in the manager array
        if(pump_control_unit == NULL){
            ESP_LOGE("APP_MAIN", "Failed to create pump control unit instance");
            return SYSTEM_FAILED;
        }
        err = pump_control_unit_init(pump_control_unit);
        if(err != SYSTEM_OK){
            ESP_LOGE("APP_MAIN", "Failed to initialize pump control unit instance with error code: %d", err);
            return SYSTEM_FAILED;
        }
        ESP_LOGI("APP_MAIN", "Running pump monitor loop for pump control unit at index %d with pump control unit instance pointer: %p", i, pump_control_unit);
              
        cJSON* pump_control_unit_json = cJSON_GetArrayItem(pcus_json, i);
        if(pump_control_unit_json == NULL){
            ESP_LOGE("APP_MAIN", "Failed to get pump control unit JSON object at index %d", i);
            continue;
        }
        int num_of_pumps=0;
        err = get_pumps_configs_from_pump_control_json(pump_control_unit_json, &pump_configs,&num_of_pumps);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d pump configs from pump control unit JSON at index %d", num_of_pumps, i);
            for(int j = 0; j < num_of_pumps; j++){
                ESP_LOGI("APP_MAIN", "Pump Config %d for Pump Control Unit %d: ID=%d, Make=%s\n", j+1, i+1, pump_configs[j].pump_id, pump_configs[j].pump_make);
                pump_t* pump_instance = NULL;
                err = setup_pump_from_config(&pump_configs[j], &pump_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup pump instance from config for pump ID %d in pump control unit JSON at index %d with error code: %d", pump_configs[j].pump_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&pump_map, pump_configs[j].pump_id)){ 
                    ESP_LOGW("APP_MAIN", "Pump ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", pump_configs[j].pump_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&pump_map, pump_configs[j].pump_id, (void*)pump_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add pump instance to the map for pump ID %d in pump control unit JSON at index %d", pump_configs[j].pump_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get pump configs from pump control unit JSON at index %d with error code: %d", i, err);     
        }
        int num_sensors = 0;
        err = get_current_sensors_configs_from_pump_control_json(pump_control_unit_json,&current_sensor_configs, &num_sensors);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d current sensor configs from pump control unit JSON at index %d", num_sensors, i);
            for(int j = 0; j < num_sensors; j++){
                ESP_LOGI("APP_MAIN", "Current Sensor Config %d for Pump Control Unit %d: ID=%d\n", j+1, i+1, current_sensor_configs[j].current_sensor_id);
                current_sensor_t* current_sensor_instance = NULL;
                err = setup_current_sensor_from_config(&current_sensor_configs[j], &current_sensor_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup current sensor instance from config for sensor ID %d in pump control unit JSON at index %d with error code: %d", current_sensor_configs[j].current_sensor_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&current_sensor_map, current_sensor_configs[j].current_sensor_id)){ 
                    ESP_LOGW("APP_MAIN", "Current Sensor ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", current_sensor_configs[j].current_sensor_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&current_sensor_map, current_sensor_configs[j].current_sensor_id, (void*)current_sensor_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add pump instance to the map for pump ID %d in pump control unit JSON at index %d", pump_configs[j].pump_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
                ESP_LOGI("APP_MAIN", "Successfully added current sensor instance to the map for sensor ID %d in pump control unit JSON at index %d, value stored: %p", current_sensor_configs[j].current_sensor_id, i,current_sensor_instance);
            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get current sensor configs from pump control unit JSON at index %d with error code: %d", i, err);     
        }
        int num_relays = 0;
        err = get_relays_configs_from_pump_control_json(pump_control_unit_json, &relay_configs, &num_relays);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d relay configs from pump control unit JSON at index %d", num_relays, i);
            for(int j = 0; j < num_relays; j++){
                ESP_LOGI("APP_MAIN", "Relay Config %d for Pump Control Unit %d: ID=%d, Pin=%d\n", j+1, i+1, relay_configs[j].relay_id, relay_configs[j].relay_pin_number);
                relay_t* relay_instance = NULL;
                err = setup_relay_from_config(&relay_configs[j], &relay_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup relay instance from config for relay ID %d in pump control unit JSON at index %d with error code: %d", relay_configs[j].relay_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&relay_map, relay_configs[j].relay_id)){ 
                    ESP_LOGW("APP_MAIN", "Relay ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", relay_configs[j].relay_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&relay_map, relay_configs[j].relay_id, (void*)relay_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add relay instance to the map for relay ID %d in pump control unit JSON at index %d", relay_configs[j].relay_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
                ESP_LOGI("APP_MAIN", "Successfully added relay instance to the map for relay ID %d in pump control unit JSON at index %d, value stored: %p", relay_configs[j].relay_id, i,relay_instance);
            }

        }else{
            ESP_LOGE("APP_MAIN", "Failed to get relay configs from pump control unit JSON at index %d with error code: %d", i, err);     
        }


        //setup pump_monitors
        int num_pump_monitors = 0;
        err = get_pump_monitors_configs_from_pump_control_json(pump_control_unit_json,&pump_monitor_configs, &num_pump_monitors);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d pump monitor configs from pump control unit JSON at index %d", num_pump_monitors, i);
            for(int j = 0; j < num_pump_monitors; j++){
                ESP_LOGI("APP_MAIN", "Pump Monitor Config %d for Pump Control Unit %d: ID=%d\n", j+1, i+1, pump_monitor_configs[j].pump_monitor_id);
                pump_monitor_t* pump_monitor_instance = NULL;
                // get the pump and current sensor instances for this pump monitor from the maps using the IDs in the config
                MapEntry* map_entry = emhashmap_get(&pump_map, pump_monitor_configs[j].pump_id);
                if(map_entry == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find pump instance for pump monitor ID %d in pump control unit JSON at index %d. Pump ID: %d", pump_monitor_configs[j].pump_monitor_id, i, pump_monitor_configs[j].pump_id);
                }
                pump_t* associated_pump = (pump_t*)map_entry->value;
                map_entry = emhashmap_get(&current_sensor_map, pump_monitor_configs[j].current_sensor_id);
                if(map_entry == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find current sensor instance for pump monitor ID %d in pump control unit JSON at index %d. Current Sensor ID: %d", pump_monitor_configs[j].pump_monitor_id, i, pump_monitor_configs[j].current_sensor_id);
                }
                current_sensor_t* associated_current_sensor = (current_sensor_t*)map_entry->value;
                if(associated_pump == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find associated pump instance for pump monitor ID %d in pump control unit JSON at index %d. Pump ID: %d", pump_monitor_configs[j].pump_monitor_id, i, pump_monitor_configs[j].pump_id);
                    continue;
                }
                if(associated_current_sensor == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find associated current sensor instance for pump monitor ID %d in pump control unit JSON at index %d. Current Sensor ID: %d", pump_monitor_configs[j].pump_monitor_id, i, pump_monitor_configs[j].current_sensor_id);
                    continue;
                }
                // print pointer values for debugging
                ESP_LOGI("APP_MAIN", "Associated pump instance pointer for pump monitor ID %d in pump control unit JSON at index %d: %p", pump_monitor_configs[j].pump_monitor_id, i, associated_pump);
                ESP_LOGI("APP_MAIN", "Associated current sensor instance pointer for pump monitor ID %d in pump control unit JSON at index %d: %p", pump_monitor_configs[j].pump_monitor_id, i, associated_current_sensor);
                err = setup_pump_monitor_from_config(&pump_monitor_configs[j],&pump_monitor_instance, associated_pump, associated_current_sensor, twenty_percent_min_analytics_callback);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup pump monitor instance from config for monitor ID %d in pump control unit JSON at index %d with error code: %d", pump_monitor_configs[j].pump_monitor_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&pump_monitor_map, pump_monitor_configs[j].pump_monitor_id)){ 
                    ESP_LOGW("APP_MAIN", "Pump Monitor ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", pump_monitor_configs[j].pump_monitor_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&pump_monitor_map, pump_monitor_configs[j].pump_monitor_id, (void*)pump_monitor_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add pump monitor instance to the map for monitor ID %d in pump control unit JSON at index %d", pump_monitor_configs[j].pump_monitor_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
                err = pump_control_unit_add_pump_monitor(pump_control_unit, pump_monitor_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to add pump monitor instance to pump control unit for monitor ID %d in pump control unit JSON at index %d with error code: %d", pump_monitor_configs[j].pump_monitor_id, i, err);
                    continue;
                }
                ESP_LOGI("APP_MAIN", "Successfully added pump monitor instance to the map for monitor ID %d in pump control unit JSON at index %d, value stored: %p", pump_monitor_configs[j].pump_monitor_id, i,pump_monitor_instance);
            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get pump monitor configs from pump control unit JSON at index %d with error code: %d", i, err);
        }

        int num_level_sensors = 0;
        err = get_level_sensors_configs_from_pump_control_json(pump_control_unit_json, &level_sensor_configs, &num_level_sensors);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved level sensor config from pump control unit JSON at index %d", i);
            for(int j = 0; j < num_level_sensors; j++){
                ESP_LOGI("APP_MAIN", "Level Sensor Config %d for Pump Control Unit %d: ID=%d, Interface=%d\n", j+1, i+1, level_sensor_configs[j].level_sensor_id,level_sensor_configs[j].interface);
                // setup level sensor instance from config and add to map (not implemented in this code snippet)
                level_sensor_t* level_sensor_instance = NULL;
                if(level_sensor_configs[j].interface != RS485){
                    ESP_LOGI("APP_MAIN", "Setting up level sensor instance for sensor ID %d in pump control unit JSON at index %d NOT RS485 interface", level_sensor_configs[j].level_sensor_id, i);
                    continue; // Skip setup for non-RS485 interfaces for now, can add handling for other interfaces later
                }
                // using only rs485_one for now, others may be added later based on the interface type in the config
                err = setup_level_sensor_from_config(&level_sensor_configs[j], &level_sensor_instance, rs485_one);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup level sensor instance from config for sensor ID %d in pump control unit JSON at index %d with error code: %d", level_sensor_configs[j].level_sensor_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&level_sensor_map, level_sensor_configs[j].level_sensor_id)){ 
                    ESP_LOGW("APP_MAIN", "Level Sensor ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", level_sensor_configs[j].level_sensor_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&level_sensor_map, level_sensor_configs[j].level_sensor_id, (void*)level_sensor_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add level sensor instance to the map for sensor ID %d in pump control unit JSON at index %d", level_sensor_configs[j].level_sensor_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
                ESP_LOGI("APP_MAIN", "Successfully added level sensor instance to the map for sensor ID %d in pump control unit JSON at index %d, value stored: %p", level_sensor_configs[j].level_sensor_id, i,level_sensor_instance);
            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get level sensor config from pump control unit JSON at index %d with error code: %d", i, err);
        }

        int num_tanks = 0;
        err = get_tanks_configs_from_pump_control_json(pump_control_unit_json, &tank_configs, &num_tanks);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d tank configs from pump control unit JSON at index %d", num_tanks, i);
            for(int j = 0; j < num_tanks; j++){
                ESP_LOGI("APP_MAIN", "Tank Config %d for Pump Control Unit %d: ID=%d, Capacity=%.2f litres\n", j+1, i+1, tank_configs[j].tank_id, tank_configs[j].tank_capacity);
                // setup tank instance from config and add to map (not implemented in this code snippet)
                tank_t* tank_instance = NULL;
                err = setup_tank_from_config(&tank_configs[j], &tank_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup tank instance from config for tank ID %d in pump control unit JSON at index %d with error code: %d", tank_configs[j].tank_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&tank_map, tank_configs[j].tank_id)){ 
                    ESP_LOGW("APP_MAIN", "Tank ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", tank_configs[j].tank_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&tank_map, tank_configs[j].tank_id, (void*)tank_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add tank instance to the map for tank ID %d in pump control unit JSON at index %d", tank_configs[j].tank_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get tank configs from pump control unit JSON at index %d with error code: %d", i, err);
        }

        int num_tank_monitors = 0;
        err = get_tank_monitors_configs_from_pump_control_json(pump_control_unit_json, &tank_monitor_configs, &num_tank_monitors);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d tank monitor configs from pump control unit JSON at index %d", num_tank_monitors, i);
            for(int j = 0; j < num_tank_monitors; j++){
                ESP_LOGI("APP_MAIN", "Tank Monitor Config %d for Pump Control Unit %d: ID=%d\n", j+1, i+1, tank_monitor_configs[j].tank_monitor_id);
                // setup tank monitor instance from config and add to map (not implemented in this code snippet)
                tank_monitor_t* tank_monitor_instance = NULL;
                // get the tank and level sensor instances for this tank monitor from the maps using the IDs in the config
                MapEntry* map_entry = emhashmap_get(&tank_map, tank_monitor_configs[j].tank_id);
                if(map_entry == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find tank instance for tank monitor ID %d in pump control unit JSON at index %d. Tank ID: %d", tank_monitor_configs[j].tank_monitor_id, i, tank_monitor_configs[j].tank_id);
                }
                tank_t* associated_tank = (tank_t*)map_entry->value;
                map_entry = emhashmap_get(&level_sensor_map, tank_monitor_configs[j].level_sensor_id);
                if(map_entry == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find level sensor instance for tank monitor ID %d in pump control unit JSON at index %d. Level Sensor ID: %d", tank_monitor_configs[j].tank_monitor_id, i, tank_monitor_configs[j].level_sensor_id);
                }
                level_sensor_t* associated_level_sensor = (level_sensor_t*)map_entry->value;
                if(associated_tank == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find associated tank instance for tank monitor ID %d in pump control unit JSON at index %d. Tank ID: %d", tank_monitor_configs[j].tank_monitor_id, i, tank_monitor_configs[j].tank_id);
                    continue;
                }
                if(associated_level_sensor == NULL){
                    ESP_LOGE("APP_MAIN", "Failed to find associated level sensor instance for tank monitor ID %d in pump control unit JSON at index %d. Level Sensor ID: %d", tank_monitor_configs[j].tank_monitor_id, i, tank_monitor_configs[j].level_sensor_id);
                    continue;
                }
                // print pointer values for debugging
                ESP_LOGI("APP_MAIN", "Associated tank instance pointer for tank monitor ID %d in pump control unit JSON at index %d: %p", tank_monitor_configs[j].tank_monitor_id, i, associated_tank);
                ESP_LOGI("APP_MAIN", "Associated level sensor instance pointer for tank monitor ID %d in pump control unit JSON at index %d: %p", tank_monitor_configs[j].tank_monitor_id, i, associated_level_sensor);
                err = setup_tank_monitor_from_config(&tank_monitor_configs[j],&tank_monitor_instance, associated_tank, associated_level_sensor, level_sensor_average_analytics_callback);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to setup tank monitor instance from config for monitor ID %d in pump control unit JSON at index %d with error code: %d", tank_monitor_configs[j].tank_monitor_id, i, err);
                    continue;
                }
                if(emhashmap_contains(&tank_monitor_map, tank_monitor_configs[j].tank_monitor_id)){ 
                    ESP_LOGW("APP_MAIN", "Tank Monitor ID %d already exists in the map, skipping addition for pump control unit JSON at index %d", tank_monitor_configs[j].tank_monitor_id, i);
                    continue; // Skip adding to the map if pump ID already exists
                }
                if (!emhashmap_put(&tank_monitor_map, tank_monitor_configs[j].tank_monitor_id, (void*)tank_monitor_instance)){
                    ESP_LOGE("APP_MAIN", "Failed to add tank monitor instance to the map for monitor ID %d in pump control unit JSON at index %d", tank_monitor_configs[j].tank_monitor_id, i);
                    return SYSTEM_FAILED; // Handle error in adding to the map
                }
                ESP_LOGI("APP_MAIN", "Successfully added tank monitor instance to the map for monitor ID %d in pump control unit JSON at index %d, value stored: %p", tank_monitor_configs[j].tank_monitor_id, i,tank_monitor_instance);
                err = pump_control_unit_add_tank_monitor(pump_control_unit, tank_monitor_instance);
                if(err != SYSTEM_OK){
                    ESP_LOGE("APP_MAIN", "Failed to add tank monitor instance to tank control unit for monitor ID %d in tank control unit JSON at index %d with error code: %d", tank_monitor_configs[j].tank_monitor_id, i, err);
                    continue;
                }
                ESP_LOGI("APP_MAIN", "Successfully added tank monitor instance to the map for monitor ID %d in tank control unit JSON at index %d, value stored: %p", tank_monitor_configs[j].tank_monitor_id, i,tank_monitor_instance);

            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get tank monitor configs from pump control unit JSON at index %d with error code: %d", i, err);
        }

        int num_subscriptions = 0;
        err = get_subscriptions_configs_from_pump_control_json(pump_control_unit_json, &subscription_configs, &num_subscriptions);
        if(err == SYSTEM_OK){
            ESP_LOGI("APP_MAIN", "Successfully retrieved %d subscription configs from pump control unit JSON at index %d", num_subscriptions, i);
            for(int j = 0; j < num_subscriptions; j++){
                if(subscription_configs[j].monitor_type == PUMP_MONITOR){ // handling pump monitor type for now, can add more types later
                    MapEntry* map_entry = emhashmap_get(&pump_monitor_map, subscription_configs[j].monitor_id);
                    if(map_entry == NULL){
                        ESP_LOGE("APP_MAIN", "Failed to find pump monitor instance for subscription config at index %d in pump control unit JSON at index %d. Monitor ID: %d", j, i, subscription_configs[j].monitor_id);
                        continue;
                    }
                    pump_monitor_t* pump_monitor_instance = (pump_monitor_t*)map_entry->value;
                    int number_of_subscribers = subscription_configs[j].num_subscribers;
                    for(int k = 0; k < number_of_subscribers; k++){
                        subscriber_t* subscriber_instance = &subscription_configs[j].subscribers[k];
                        if(subscriber_instance->type == SUBSCRIBER_TYPE_RELAY){ // handling relay type for now, can add more types later
                            MapEntry* relay_map_entry = emhashmap_get(&relay_map, subscriber_instance->id);
                            if(relay_map_entry == NULL){
                                ESP_LOGE("APP_MAIN", "Failed to find relay instance for subscriber ID %d in subscription config at index %d in pump control unit JSON at index %d. Relay ID: %d", subscriber_instance->id, j, i, subscriber_instance->id);
                                continue;
                            }
                            relay_t* relay_instance = (relay_t*)relay_map_entry->value;
                            relay_response_type_t response_type = subscription_configs[j].subscribers[k].response.relay_response; 
                            pump_monitor_event_callback_t event_callback = get_relay_response_callback(response_type);
                            pump_monitor_subscriber_t pump_monitor_subscriber = {
                                .id = subscriber_instance->id,
                                .context = (void*)relay_instance,
                                .callback = event_callback
                            };
                            int event_id;
                            error_type_t subscribe_err = pump_monitor_subscribe_event(pump_monitor_instance, &pump_monitor_subscriber, &event_id);
                            if(subscribe_err != SYSTEM_OK){
                                ESP_LOGE("APP_MAIN", "Failed to subscribe to pump monitor events for subscription config at index %d in pump control unit JSON at index %d with error code: %d", j, i, subscribe_err);
                                continue;
                            }
                            ESP_LOGI("APP_MAIN", "Successfully subscribed to pump monitor events for subscription config at index %d in pump control unit JSON at index %d with event ID: %d", j, i, event_id);
                        }
                    }
                    ESP_LOGI("APP_MAIN", "Finished processing subscription config at index %d in pump control unit JSON at index %d for monitor type %d with monitor ID %d", j, i, subscription_configs[j].monitor_type, subscription_configs[j].monitor_id);
                }
                else if(subscription_configs[j].monitor_type == TANK_MONITOR){
                    // handling tank monitor type for now, can add more types later
                    MapEntry* map_entry = emhashmap_get(&tank_monitor_map, subscription_configs[j].monitor_id);
                    if(map_entry == NULL){
                        ESP_LOGE("APP_MAIN", "Failed to find tank monitor instance for subscription config at index %d in pump control unit JSON at index %d. Monitor ID: %d", j, i, subscription_configs[j].monitor_id);
                        continue;
                    }
                    tank_monitor_t* tank_monitor_instance = (tank_monitor_t*)map_entry->value;
                    int number_of_subscribers = subscription_configs[j].num_subscribers;
                    for(int k = 0; k < number_of_subscribers; k++){
                        subscriber_t* subscriber_instance = &subscription_configs[j].subscribers[k];
                        if(subscriber_instance->type == SUBSCRIBER_TYPE_RELAY){ // handling relay type for now, can add more types later
                            MapEntry* relay_map_entry = emhashmap_get(&relay_map, subscriber_instance->id);
                            if(relay_map_entry == NULL){
                                ESP_LOGE("APP_MAIN", "Failed to find relay instance for subscriber ID %d in subscription config at index %d in pump control unit JSON at index %d. Relay ID: %d", subscriber_instance->id, j, i, subscriber_instance->id);
                                continue;
                            }
                            relay_t* relay_instance = (relay_t*)relay_map_entry->value;
                            relay_response_type_t response_type = subscription_configs[j].subscribers[k].response.relay_response; 
                            tank_monitor_event_callback_t event_callback = get_relay_response_callback(response_type);
                            tank_monitor_subscriber_t tank_monitor_subscriber = {
                                .id = subscriber_instance->id,
                                .context = (void*)relay_instance,
                                .callback = event_callback
                            };
                            int event_id;
                            error_type_t subscribe_err = tank_monitor_subscribe_event(tank_monitor_instance, &tank_monitor_subscriber, &event_id);
                            if(subscribe_err != SYSTEM_OK){
                                ESP_LOGE("APP_MAIN", "Failed to subscribe to tank monitor events for subscription config at index %d in pump control unit JSON at index %d with error code: %d", j, i, subscribe_err);
                                continue;
                            }
                            ESP_LOGI("APP_MAIN", "Successfully subscribed to tank monitor events for subscription config at index %d in pump control unit JSON at index %d with event ID: %d", j, i, event_id);
                        }
                    }
                }
                
                else{
                    ESP_LOGW("APP_MAIN", "Subscription config at index %d in pump control unit JSON at index %d has unsupported monitor type %d, skipping", j, i, subscription_configs[j].monitor_type);
                }

            }
        }else{
            ESP_LOGE("APP_MAIN", "Failed to get subscription configs from pump control unit JSON at index %d with error code: %d", i, err);
        }
    }   
    return SYSTEM_OK;
}