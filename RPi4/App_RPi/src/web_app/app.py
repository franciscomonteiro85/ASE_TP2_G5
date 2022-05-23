from flask import Flask, render_template

app = Flask(__name__)

@app.route('/temperature/<temperature>')
def index(temperature):
    showTemperature()
    return render_template('index.html', temperature=temperature)

def showTemperature():
    return 0

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')