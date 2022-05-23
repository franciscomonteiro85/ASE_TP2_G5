from bottle import Bottle, run
from env import Env

app = Bottle()

@app.get('/t_temp')
def get_t_temp():
    return str(Env.TEMP_THRESHOLD)

@app.post('/t_temp/<temp:int>')
def theshold_temp(temp:int):
    print("@theshold_temp: temp value: "+str(temp))
    Env.TEMP_THRESHOLD = temp
