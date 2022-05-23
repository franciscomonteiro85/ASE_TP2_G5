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
def theshold_temp(temp:int):
    """
    Set threshold temperature.
    """
    print("@theshold_temp: temp value: "+str(temp))
    Env.TEMP_THRESHOLD = temp
