/* LEDC (LED Controller) basic example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "aledc.h"
#include "tc74.h"

#define TEMP_READ_INTERVAL   1000 *2 // 2s
#define LED_GPIO        5
#define THRESHOLD_TEMP      23

static const char *TAG = "i2c_pwm_main";

/**
 * @brief Converts the temperature from the sensor into a possible duty cycle value
 * Temperature scale: [15 - 30 ]
 * res: 273
 * 
 * @param temperature 
 * @return int 
 */
uint32_t temp_to_duty_cicle(uint8_t temperature){
    return (temperature-20) * 200;
}

void ledcontrolTask(void *pv)
{

    // Set the LEDC peripheral configuration
    aledc_init(); 
    ESP_LOGI(TAG, "LEDC initialized!");

    uint8_t* temperature = (uint8_t*)  pv;

    int delay = TEMP_READ_INTERVAL/portTICK_RATE_MS;
    uint32_t dutycycle = temp_to_duty_cicle(*temperature);

    while (1)
    {
        dutycycle = temp_to_duty_cicle(*temperature);
        ESP_LOGI(TAG, "(ledc) Temperature is : %d", *temperature);
        ESP_LOGI(TAG, "(ledc) duty cycle is : %d", dutycycle);

        if(*temperature > THRESHOLD_TEMP) {
            gpio_set_level(LED_GPIO, 1);
        } else {
            gpio_set_level(LED_GPIO, 0);
        }

        aledc_update_duty(dutycycle);
        vTaskDelay(delay);
    }

    // aledc_update_duty(0);
    vTaskDelete(NULL);
}

void readTempTask(void *pv)
{
    // setup the sensor
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C master initialized!");

    // signed integer value of 8 bits
    uint8_t* temperature_value = (uint8_t*) pv;

    // periodically read temp values from sensor and set the sensor to power saving mode
    while (1)
    {
        // set normal mode (200uA consuption)
        i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_NORM_OP_VALUE);

        vTaskDelay(250 / portTICK_RATE_MS);

        i2c_master_read_temp(I2C_MASTER_NUM, temperature_value);

        // set standby mode (5uA consuption)
        i2c_master_set_tc74_mode(I2C_MASTER_NUM, SET_STANBY_VALUE);

        ESP_LOGI(TAG, "Temperature is : %d", *temperature_value);
        
        vTaskDelay( TEMP_READ_INTERVAL / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main(void)
{

    // set up gpio pin
    gpio_config_t gpio_conf = {
        .pin_bit_mask = LED_GPIO,
        .mode = GPIO_MODE_DEF_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&gpio_conf);
    gpio_set_level(LED_GPIO, 0);

    // shared variable to store the temperature values
    // only wriiten in the logTemp task
    static uint8_t temperature_value;

    xTaskCreate(ledcontrolTask, "t_ledcontrol", 1024 * 2, &temperature_value, tskIDLE_PRIORITY, NULL);
    xTaskCreate(readTempTask, "t_readTemp", 1024 * 2, &temperature_value, tskIDLE_PRIORITY, NULL);
}