/*
* https://github.com/ThomasGeor/esp32_tc74_temperature_sensor/
*/

#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

// Defintions

// registers
#define READ_TEMP_REGISTER          0x00
#define READ_WRITE_CONFIG_REGISTER  0x01
#define SET_NORM_OP_VALUE           0x00  /*!< sets the 7th bit of configuration register to normal mode */
#define SET_STANBY_VALUE            0x80  /*!< sets the 7th bit of configuration register to standby mode */

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define TC74_SLAVE_ADDR             0x4D            /*!< slave address for TC74 sensor */
#define WRITE_BIT I2C_MASTER_WRITE                  /*!< I2C master write */
#define READ_BIT  I2C_MASTER_READ                   /*!< I2C master read */
#define ACK_CHECK_EN 0x1                            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                                 /*!< I2C ack value */
#define NACK_VAL 0x1                                /*!< I2C nack value */

// Function declarations

esp_err_t i2c_master_init();
esp_err_t i2c_master_read_temp(i2c_port_t i2c_num, uint8_t *tmprt);
esp_err_t i2c_master_set_tc74_mode(i2c_port_t i2c_num,uint8_t mode);
esp_err_t i2c_master_read_tc74_config(i2c_port_t i2c_num, uint8_t *mode);