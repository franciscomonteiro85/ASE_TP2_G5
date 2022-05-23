from bottle import Bottle, run
from env import Env

"""Small http server as to provide easy to use endpoints for configuration and RW operations.
Meant for internal use due to not implmenting any security features. 
An external server may be used as proxy. 
"""

app = Bottle()

@app.get('/temp')
def get_t_temp():
    """
    Get current threshold temperature.
    """
    return str(Env.TEMP)

@app.get('/t_temp')
def get_t_temp():
    """
    Get current threshold temperature.
    """
    return str(Env.TEMP_THRESHOLD)

@app.post('/t_temp/<temp:int>')
def threshold_temp(temp:int):
    """
    Set threshold temperature.
    """
    print("@threshold_temp: temp value: "+str(temp))
    Env.TEMP_THRESHOLD = temp

@app.post('/t_step/<step:int>')
def duty_cycle_step(step:int):
    """
    Set duty cycle step.
    """
    print("@duty_cycle_step: step value: "+str(step))
    Env.DUTY_CYCLE_STEP = step

@app.post('/t_start_temp/<start_temp:int>')
def start_temperature(start_temp:int):
    """
    Set start temperature.
    """
    print("@start_temp: start temperature value: "+str(start_temp))
    Env.START_TEMPERATURE = start_temp