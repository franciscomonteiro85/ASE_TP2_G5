class Env:
    
    GPIO_LED_NR = 37
    PWM_LED_NR = 38
    PWM_LED_FREQ = 1000


    I2C_BUS = 1
    TC74_I2C_ADD = 0x4D

    E2PROM_CE_NR = 0
    E2PROM_SPI_CH = 0

    # shared variables 
    TEMP_THRESHOLD = 20
    TEMP = -1
    DUTY_CYCLE_STEP = 0.1
    START_TEMPERATURE = 20

