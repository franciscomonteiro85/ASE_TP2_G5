/* 
* Some code by Paulo Pinho  - ASE_TP2_G6
* https://github.com/CPinhoK/ASE_TP2_G6
*/



#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "eeprom.h"

#include <driver/dac.h>
#include <driver/dac_common.h>
#include "driver/gpio.h"

 
int main ()
{
    time_t seconds;
     
    seconds = time(NULL);
    printf("Seconds since January 1, 1970 = %ld\n", seconds);
     
    return(0);
}




/*
 This code demonstrates how to use the SPI master half duplex mode to read/write a AT932C46D EEPROM (8-bit mode).
*/

#   define EEPROM_HOST    VSPI_HOST
#   define PIN_NUM_MISO 19
#   define PIN_NUM_MOSI 23
#   define PIN_NUM_CLK  18
#   define PIN_NUM_CS   13


#define WR_LED_GPIO 25 

static const char TAG[] = "main";

void dump(uint8_t *dt, int n);
void write_data(EEPROM_t dev, char in);

void app_main(void)
{

    //Initialize DAC
    dac_output_enable(DAC_CHANNEL_1); //ativar a saida da DAC no GPIO25
    //dac_output_enable(DAC_CHANNEL_2); //ativar a saida da DAC no GPIO26

	// writeing goip led
	gpio_reset_pin(WR_LED_GPIO);
	gpio_set_direction(WR_LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(WR_LED_GPIO, 0);


    //Initialize the EEPROM

    ESP_LOGI(TAG, "EEPROM_MODEL=%s", "25LC040A 4-Kbit SPI Bus Serial EEPROM");
	EEPROM_t dev;
	spi_master_init(&dev);
	int32_t totalBytes = eeprom_TotalBytes(&dev);
	ESP_LOGI(TAG, "totalBytes=%d Bytes",totalBytes);

	
	// Write Data
	uint8_t wdata[512];
	// Read Data
	uint8_t rbuf[512];
	// Buffer Length
	int len;
	int skip=1;

	srand(time(NULL));   // Initialization, should only be called once.
	
	gpio_set_level(WR_LED_GPIO, 1);
	for (int addr=0; addr<512;addr++) {

		len =  eeprom_WriteByte(&dev, addr, 0x65);
		ESP_LOGD(TAG, "WriteByte(addr=%d) len=%d", addr, len);
		if (len != 1) {
			ESP_LOGE(TAG, "WriteByte Fail addr=%d", addr);
			while(1) { vTaskDelay(1); }
		}
	}

	gpio_set_level(WR_LED_GPIO, 0);


	ESP_LOGI(TAG, "Read Memory");
	// Read 512 byte from Address=0
	memset(rbuf, 0, 512);
	len =  eeprom_Read(&dev, 0, rbuf, 512);
	if (len != 512) {
		ESP_LOGE(TAG, "Read Fail");
		while(1) { vTaskDelay(1); }
	}
	ESP_LOGI(TAG, "Read Data: len=%d", len);
	dump(rbuf, 512);

    //set write dac output voltage
    int i = 0;

    for (i=0; i < 512; i++){
        dac_output_voltage(DAC_CHANNEL_1, rbuf[i]);
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
    dac_output_voltage(DAC_CHANNEL_1, 0);


    while (1) {
        // Add your main loop handling code here.
        vTaskDelay(1);
    }
}

void dump(uint8_t *dt, int n)
{
	uint16_t clm = 0;
	uint8_t data;
	uint32_t saddr =0;
	uint32_t eaddr =n-1;

	printf("--------------------------------------------------------\n");
	uint32_t addr;
	for (addr = saddr; addr <= eaddr; addr++) {
		data = dt[addr];
		if (clm == 0) {
			printf("%05x: ",addr);
		}

		printf("%02x ",data);
		clm++;
		if (clm == 16) {
			printf("| \n");
			clm = 0;
		}
	}
	printf("--------------------------------------------------------\n");
}