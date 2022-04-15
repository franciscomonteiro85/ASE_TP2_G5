/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

//define uart
#define ECHO_UART_BAUD_RATE (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define BUF_SIZE    (1024)
#define UART1_GPIO_RX   (4)
#define UART1_GPIO_TX   (5)

//define adc
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64         

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

uint32_t adc_reading = 0;

static void uart1_task(void *arg)
{
    while (1) {
        uint8_t i = adc_reading;
        uart_write_bytes(UART_NUM_1, &i, 4*4);
        
        printf("UART1 sending to UART2: %d\n", i);
        //sleep for 1000 ms
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    //Configure UART
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

    uart_set_sw_flow_ctrl(UART_NUM_1, true, 0, 0);

    esp_rom_gpio_connect_out_signal(UART1_GPIO_RX, UART_PERIPH_SIGNAL(2, SOC_UART_TX_PIN_IDX), false, false);
    //UART1 RX -> UART1_PIN_RX
    esp_rom_gpio_connect_in_signal(UART1_GPIO_RX, UART_PERIPH_SIGNAL(1, SOC_UART_RX_PIN_IDX), false);

    //UART1 TX -> UART1_GPIO_TX
    esp_rom_gpio_connect_out_signal(UART1_GPIO_TX, UART_PERIPH_SIGNAL(1, SOC_UART_TX_PIN_IDX), false, false);
    //UART2 RX -> UART1_GPIO_TX
    esp_rom_gpio_connect_in_signal(UART1_GPIO_TX, UART_PERIPH_SIGNAL(2, SOC_UART_RX_PIN_IDX), false);

    //Configure GPIO
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << TURN_OFF_GPIO;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //ADC Readings
    while (1) {
        adc_reading = adc1_get_raw((adc1_channel_t)channel);

        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    xTaskCreate(uart1_task, "uart1_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
