from flask import Flask, make_response, render_template
import requests

app = Flask(__name__)

@app.route('/temperature')
def index():
    return render_template('index.html')

@app.route('/get_temperature')
def get_temperature():
    temperature = requests.get("http://localhost:8080/temp")
    return make_response({"output": int(temperature.content)})

@app.route('/set_threshold/<threshValue>', methods=['POST'])
def set_threshold(threshValue):
    requests.post("http://localhost:8080/t_temp/" + threshValue)
    return threshValue

@app.route('/set_duty_cycle/<stepValue>', methods=['POST'])
def set_duty_cycle(stepValue):
    requests.post("http://localhost:8080/t_step/" + stepValue)
    return stepValue

@app.route('/set_start_temp/<start_temp>', methods=['POST'])
def set_start_temp(start_temp):
    requests.post("http://localhost:8080/t_start_temp/" + start_temp)
    return start_temp

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')