import spidev

_SPI_SPEED_HZ = 5000000 # max speed = 5MHz ( 2.5V <= Vcc <= 4.5V )

_RD_INS = 0x03
_WR_INS = 0x02

class E2PROM:
    """Helper class to handle the spi e2prom.
        """

    def __init__(self, spi_bus:int, spi_cs:int) -> None:
        """Helper class to handle the spi e2prom.

        Args:
            spi_bus (int): Spi bus to use
            spi_cs (int): what cs channel to use
        """
        self.spi = spidev.SpiDev(spi_bus, spi_cs)
        self.spi.max_speed_hz = _SPI_SPEED_HZ
        # not writable, but 
        # self.spi.cshigh = False # CS is active low

    def close(self):
        """Disconnect from the spi device.
        """
        self.spi.close()

    def write_byte(self, address, byte):
        if not self.check_address(address):
            raise ValueError

        values = []

        # first byte: instruction + msb
        # second byte: address (8 less significant bits)
        if(address>255):
            values.append(_WR_INS | 0x08)
            address-= 255
        else:
            values.append(_WR_INS)

        values.append(address)

        # third byte: byte to be written in memory
        values.append(byte)

        # write to memory
        self.spi.xfer2(values)
    
        
    def read_data(self, address):
        if not self.check_address(address):
            raise ValueError

        values = []

        # first byte: instruction + adress msb
        # second byte: address (8 less significant bits)
        if(address>255):
            values.append(_RD_INS | 0x08)
            address-= 255
        else:
            values.append(_RD_INS)

        values.append(address)

        # third byte: return value
        rtvl = self.spi.xfer2(values)

        return rtvl

    def check_address(address):
        """Check if a given address is in the memory address range

        Args:
            address (int): address to be checked

        Returns:
            boolean: true if adress is in range else false
        """
        return address >0 and address <512 
