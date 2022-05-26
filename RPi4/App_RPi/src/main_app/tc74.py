"""
Code for a handler class of TC74 temperature sensor.
Based on code for a C lib to handle the same sensor:
https://github.com/ThomasGeor/esp32_tc74_temperature_sensor/
"""

from env import Env
from smbus2 import SMBus

_READ_TEMP_REG = 0
_CONFIG_REG = 1

_CFG_STANDBY_MODE = 0X80
_CFG_NORMAL_MODE = 0X00

class TC74:
    """
    Helper class to handle the TC74 temperature sensor
    """
    def __init__(self, bus:SMBus) -> None:
        """Helper class to handle the TC74 temperature sensor

        Args:
            bus (SMBus): SMBus object that represents a SMBus bus to be used for I2C communication with the sensor
        """
        # SMBus object for I2C bus communication
        self.bus = bus
        # device I2C bus address
        self.address = Env.TC74_I2C_ADD

    def set_mode(self, mode:int) -> int:
        """Sets the device to the given mode. Writes value to register.

        Args:
            mode (int): Mode to put device in

        Returns:
            int: 0 if successfull
        """
        self.bus.write_byte_data(self.address, _CONFIG_REG, mode)
        return 0

    def set_standby_mode(self):
        """Sets sensor to standby mode. Sensor doenst update temperature value

        Returns:
            int: 0 if successfull
        """
        return self.set_mode(_CFG_STANDBY_MODE)
    
    def set_normal_mode(self):
        """Sets sensor to normal mode (updates internal temperature value). Required to get the most up to date temprature value.

        Returns:
            int: 0 if successfull
        """
        return self.set_mode(_CFG_NORMAL_MODE)

    def read_temp(self) -> int:
        """Only reads temperature value from sensor's register.
        Doesnt change its operational mode.

        Returns:
            int: temperature value (in celsius)
        """
        return self.bus.read_byte_data(self.address, _READ_TEMP_REG)

    def read_temp_auto(self) -> int:
        """More automatic method to read sensor temperature. 
        Forces device into normal mode before reading and puts it in standy mode after reading value.

        Returns:
            int: temperature value (in celsius).
        """
        self.set_normal_mode()
        value = self.read_temp()
        self.set_standby_mode()
        return value
