/* SPI Master Half Duplex EEPROM example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/dac.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "spi_eeprom.h"


/*
 This code demonstrates how to use the SPI master half duplex mode to read/write a AT932C46D EEPROM (8-bit mode).
*/

static const char TAG[] = "main";

void app_main(void)
{

    //Initialize DAC
    dac_output_enable(DAC_CHANNEL_1); //ativar a saida da DAC no GPIO25
    dac_output_enable(DAC_CHANNEL_2); //ativar a saida da DAC no GPIO26

    //set write dac output voltage
    int i = 0;

    for (i=0; i < 255; i++){
        dac_output_voltage(DAC_CHANNEL_1, i);
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    dac_output_voltage(DAC_CHANNEL_1, 0);

    int j = 0;
    for (j=0; j< 255; j++){
        dac_output_voltage(DAC_CHANNEL_2, j); //colocar voltagem na saída do canal DAC do GPIO25 
        vTaskDelay(30 / portTICK_PERIOD_MS); //pequeno delay
    }
    dac_output_voltage(DAC_CHANNEL_2, 0);

    while (1) {
        // Add your main loop handling code here.
        vTaskDelay(1);
    }
}
