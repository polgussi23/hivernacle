from flask import Flask, request, jsonify, render_template
import serial
import threading
import time

app = Flask(__name__)

# Configuració del port sèrie
arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)

# Variable per emmagatzemar els valors dels sensors
sensor_values = {
    "temperatura": None,
    "humitat": None,
    "humitat_sòl": None,  # Nou sensor de humitat al sòl
    "lluminositat": None
}

# Estat inicial de la finestra
finestra_oberta = False

def llegir_sensors():
    """Funció que llegeix els valors dels sensors des del port sèrie."""
    global sensor_values
    while True:
        try:
            if arduino.in_waiting > 0:
                linia = arduino.readline().decode('utf-8').strip()
                # Processa les dades si tenen el format esperat
                if "Temperatura" in linia:
                    parts = linia.split(":")
                    if len(parts) > 1:
                        sensor_values["temperatura"] = parts[1].strip().replace("\u00b0C", "").strip()
                elif "Humitat" in linia and "sòl" not in linia:
                    parts = linia.split(":")
                    if len(parts) > 1:
                        sensor_values["humitat"] = parts[1].strip().replace("%", "").strip()
                elif "Humitat del sòl" in linia:
                    parts = linia.split(":")
                    if len(parts) > 1:
                        sensor_values["humitat_sòl"] = parts[1].strip().replace("%", "").strip()
                elif "Lluminositat" in linia:
                    parts = linia.split(":")
                    if len(parts) > 1:
                        sensor_values["lluminositat"] = parts[1].strip().replace("lx", "").strip()
        except Exception as e:
            print(f"Error llegint sensors: {e}")
        time.sleep(0.1)  # Evita saturar la CPU

# Executa el fil per llegir dades sèrie
sensor_thread = threading.Thread(target=llegir_sensors, daemon=True)
sensor_thread.start()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/ordre', methods=['POST'])
def enviar_ordre():
    global finestra_oberta
    data = request.json
    ordre = data.get('ordre', '').strip()

    if ordre == "finestra":
        if finestra_oberta:
            ordre_a_enviar = "tanca"
            finestra_oberta = False
        else:
            ordre_a_enviar = "obre"
            finestra_oberta = True
        arduino.write((ordre_a_enviar + '\n').encode())
        return jsonify({"status": "OK", "ordre": ordre_a_enviar})

    elif ordre in ["ventiladors", "llums", "rega"]:
        arduino.write((ordre + '\n').encode())
        return jsonify({"status": "OK", "ordre": ordre})

    else:
        return jsonify({"status": "Error", "missatge": "Ordre no reconeguda"}), 400

@app.route('/valors', methods=['GET'])
def obtenir_valors():
    """Endpoint per retornar els valors actuals dels sensors."""
    return jsonify(sensor_values)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
