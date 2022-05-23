from time import sleep

from smbus2 import SMBus
from env import Env

from gpiozero import LED, PWMLED
from e2rom import E2PROM

from tc74 import TC74

from server import app, run
import threading

# thread that rus the main loop
t:threading.Thread = None

class Components:
    def __init__(self, tc74:TC74 = None, gpio_led:LED = None, pwm_led:PWMLED = None, mem=None) -> None:
        """Holder class for the components used in the app.

        Args:
            tc74 (TC74, optional): _description_. Defaults to None.
            gpio_led (LED, optional): _description_. Defaults to None.
            pwm_led (PWMLED, optional): _description_. Defaults to None.
            mem (_type_, optional): _description_. Defaults to None.
        """
        self.tc74 = tc74
        self.gpio_led = gpio_led
        self.pwm_led = pwm_led
        self.mem = mem

def init_components():
    # todo: SPI e2prom init
    #mem = E2PROM(env.E2PROM_SPI_CH, env.E2PROM_CE_NR)

    # GPIO Led init
    led = LED(Env.GPIO_LED_NR)
    # PWM Led init
    pwm_led = PWMLED(pin=Env.PWM_LED_NR, frequency=Env.PWM_LED_FREQ)

    # initialize tx74 temp sensor
    tc74_sensor = TC74(SMBus(Env.I2C_BUS))
    # after SMB/I2C bus initialiation, there is a need for a pause to let the I2C 'settle'
    sleep(0.5)

    return Components(tc74_sensor, led, pwm_led)

def temp_to_duty_cycle(temperature: int) -> float:
    """Tranforms a temperature value into an acceptable duty cycle value: [0.0, 1.0].

    Args:
        temperature (int): temperature value.

    Returns:
        float: duty cycle in [0.0, 1.0] range.
    """
    start_temp = 20
    duty_cycle_step = 0.1
    return (temperature-start_temp)*duty_cycle_step

def toggle_gpio_led(led:LED, temp: int):
    """_summary_

    Args:
        led (LED): _description_
        temp (int): _description_
    """

    threshold_temp  = env.TEMP_THRESHOLD

    if(temp > threshold_temp):
        led.on()
    else:
        led.off()

def main():
    #initializa all components
    print("@main: initing components")
    comps = init_components()
    print("@main: components initialized")

    
    # main loop
    while(True):
        # read sensor temperature and put it in stanby until next read
        temp_value = comps.tc74.read_temp()
        print("@main: temp value: "+str(temp_value))

        #write to shared cvariable
        Env.TEMP = temp_value
        
        # update pwm powered led duty cycle 
        duty_cycle = temp_to_duty_cycle(temp_value)
        print("@main: duty cycle: "+str(duty_cycle))

        comps.pwm_led.value = duty_cycle

        # togle gpio led based on temperature value
        toggle_gpio_led(comps.gpio_led, temp_value)

        sleep(1)


if __name__ == "__main__":
    # spin main loop thread
    #t = threading.Thread(target=main)
    #t.start()
    main()
    # start server
    #run(app, host='localhost', port=8080)