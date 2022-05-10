#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#include "sdkconfig.h"
#include "esp_log.h"

static const char TAG[] = "e2rom";


#define ADDR_MASK   0x7f

#define CMD_EWDS    0x200
#define CMD_WRAL    0x200
#define CMD_ERAL    0x200
#define CMD_EWEN    0x200
#define CMD_CKBS    0x000

#define CMD_ERASE   0x380
#define CMD_WRITE   0x280

#define ADD_EWDS    0x00
#define ADD_WRAL    0x20
#define ADD_ERAL    0x40
#define ADD_EWEN    0x60

// ------------------- //
#define CMD_READ    0x03
#define CMD_WRITE   0x02
#define CMD_WREN    0x06

#define EEPROM_BUSY_TIMEOUT_MS  5

#define EEPROM_CLK_FREQ         SPI_MASTER_FREQ_8M  
#define EEPROM_INPUT_DELAY_NS   ((1000*1000*1000/EEPROM_CLK_FREQ)/2+20)

#define PIN_NUM_CS   13


static spi_device_handle_t spi;

esp_err_t e2rom_init(const eeprom_config_t *cfg, spi_device_handle_t* spi_handle ) {

    esp_err_t err = ESP_OK;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = EEPROM_CLK_FREQ,
        .mode = 0,          //SPI mode 0
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1
        };


    //Attach the EEPROM to the SPI bus
    ESP_LOGI(TAG, "Attaching device...");
    err = spi_bus_add_device(cfg->host, &devcfg, spi_handle);

    if  (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t eeprom_simple_cmd(uint8_t cmd)
{
    spi_transaction_t t = {
        .cmd = cmd
    };
    return spi_device_polling_transmit(spi, &t);
}

esp_err_t e2rom_read(uint8_t addr, uint8_t* out_data) {

    spi_transaction_t t = {
        .cmd = CMD_READ ,
        .rxlength = 8,
        .flags = SPI_TRANS_USE_RXDATA,
    };

    esp_err_t err = spi_device_polling_transmit(spi, &t);
    if (err!= ESP_OK) return err;

    *out_data = t.rx_data[0];
    return ESP_OK;
}

esp_err_t e2rom_write_disable() {
    return ESP_OK;
}

esp_err_t e2rom_write_enable() {
    return eeprom_simple_cmd(CMD_WREN);
}

esp_err_t e2rom_write(uint8_t addr, uint8_t data) {
    esp_err_t err;
    err = spi_device_acquire_bus(spi, portMAX_DELAY);
    if (err != ESP_OK) return err;

    spi_transaction_t t = {
        .cmd = CMD_WRITE,
        .length = 8,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = {data}
    };
    err = spi_device_polling_transmit(spi, &t);

    spi_device_release_bus(spi);
    return err;
}