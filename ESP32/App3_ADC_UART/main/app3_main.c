#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_rom_gpio.h"
#include "soc/uart_periph.h"

#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_log.h"

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
#define ECHO_UART_BAUD_RATE 115200
#define BUF_SIZE    (1024)
#define UART1_GPIO_RX   (4)
#define UART1_GPIO_TX   (5)
#define TURN_OFF_GPIO     (36)

//define adc
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64         

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     //GPIO36 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

uint32_t adc_reading = 0;

static void uart1_task(void *arg)
{
    while (1) {
        uint8_t i = adc_reading;
        uart_write_bytes(UART_NUM_1, &i, 4*4);
        
        printf("UART1: %d\n", i);
        //sleep for 1000 ms
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void uart2_task(void *arg)
{
    unsigned int count = 0;
    while (1) 
    {
        uint8_t data[128];
        int size = 0;

        if(size != 0) 
        {
            uart_read_bytes(UART_NUM_2, data, size, 20 / portTICK_RATE_MS);
            uint8_t *b;
            b = &data;
            printf("UART2: %d - %d\n", count, (int)*b);
            count++;
        }
    }
    //sleep for 100 ms
    vTaskDelay(100 / portTICK_PERIOD_MS);
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

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

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
    //esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    uint32_t previous_reading = 0;

    //ADC Readings
    while (1) 
    {
        previous_reading = adc_reading;
        adc_reading = adc1_get_raw((adc1_channel_t)channel);

        if (adc_reading == previous_reading)
        {
            esp_sleep_enable_timer_wakeup(3000000);  //3 seconds of sleep
            printf("Entering light sleep\n");
            uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);

            // Get timestamp before entering sleep 
            int64_t t_before_us = esp_timer_get_time();

            // Enter sleep mode
            esp_light_sleep_start();

            // Get timestamp after waking up from sleep
            int64_t t_after_us = esp_timer_get_time();

            printf("Slept for %lld ms\n", (t_after_us - t_before_us) / 1000);
           
        } else
        {
            //Convert adc_reading to voltage in mV
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
            vTaskDelay(pdMS_TO_TICKS(1000));
            xTaskCreate(uart1_task, "uart1_task", 2048, NULL, 10, NULL);
            xTaskCreate(uart2_task, "uart2_task", 2048, NULL, 10, NULL);
        }
    }
}