#include <Servo.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_VEML7700.h>

// Instàncies dels sensors
Adafruit_AHTX0 aht10;
Adafruit_VEML7700 veml7700;

Servo servoMotor;

// Definim els pins
const int ventiladorPin = 9;
const int servoPin = 3;
const int llumsPin = 5;       // Pin per a les llums
const int humitatSolPin = A3; // Pin per al sensor d'humitat del sòl
const int bombaPin = 10;      // Pin per a la bomba d'aigua

// Estat inicial
bool ventiladorsEnc = false;
bool servoObert = false;     // Indica si el servo està obert
bool llumsEnceses = false;   // Estat de les llums
bool bombaEnc = false;       // Estat de la bomba

// Temporitzador per a les lectures
unsigned long ultimaLectura = 0;
const unsigned long intervalLectures = 15000; // 15 segons

// Estats dels sensors
bool aht10Funcionant = true;
bool veml7700Funcionant = true;

void setup() {
  // Configuració dels pins
  pinMode(ventiladorPin, OUTPUT);
  digitalWrite(ventiladorPin, LOW);

  pinMode(llumsPin, OUTPUT); 
  digitalWrite(llumsPin, LOW);

  pinMode(bombaPin, OUTPUT); 
  digitalWrite(bombaPin, LOW);

  servoMotor.attach(servoPin);
  servoMotor.write(0); // Posició inicial del servo (tancat)

  // Inicia la comunicació sèrie
  Serial.begin(9600);

  // Inicialitza els sensors
  inicialitzaAHT10();
  inicialitzaVEML7700();
}

void loop() {
  // Llegeix ordres de la comunicació sèrie
  gestionaOrdres();

  // Lectura dels sensors cada 15 segons
  if (millis() - ultimaLectura >= intervalLectures) {
    ultimaLectura = millis();

    if (aht10Funcionant) {
      sensors_event_t humidity, temp;
      if (aht10.getEvent(&humidity, &temp)) {
        Serial.print("Temperatura: ");
        Serial.print(temp.temperature);
        Serial.println(" °C");

        Serial.print("Humitat: ");
        Serial.print(humidity.relative_humidity);
        Serial.println(" %");
      } else {
        Serial.println("Error llegint AHT10. Reintentant inicialització.");
        aht10Funcionant = inicialitzaAHT10();
      }
    }

    if (veml7700Funcionant) {
      float lux = veml7700.readLux();
      if (lux != 0) {
        Serial.print("Lluminositat: ");
        Serial.print(lux);
        Serial.println(" lx");
      } else {
        Serial.println("Error llegint VEML7700. Reintentant inicialització.");
        veml7700Funcionant = inicialitzaVEML7700();
      }
    }

    // Llegeix el sensor d'humitat del sòl
    int valorHumitatSol = analogRead(humitatSolPin);
    int percentatgeHumitatSol = map(valorHumitatSol, 1023, 0, 0, 100);
    Serial.print("Humitat del sòl: ");
    Serial.print(percentatgeHumitatSol);
    Serial.println(" %");

    Serial.println("-----");
  }
}

void gestionaOrdres() {
  if (Serial.available() > 0) {
    String ordre = Serial.readStringUntil('\n');
    ordre.trim();

    if (ordre == "ventiladors") {
      ventiladorsEnc = !ventiladorsEnc;
      digitalWrite(ventiladorPin, ventiladorsEnc ? HIGH : LOW);
      Serial.println(ventiladorsEnc ? "Ventiladors encesos." : "Ventiladors apagats.");
    } else if (ordre == "obre") {
      if (!servoObert) {
        for (int angle = 0; angle <= 30; angle++) {
          servoMotor.write(angle);
          delay(50);
        }
        servoObert = true;
        Serial.println("Servo obert.");
      } else {
        Serial.println("El servo ja està obert.");
      }
    } else if (ordre == "tanca") {
      if (servoObert) {
        for (int angle = 30; angle >= 0; angle--) {
          servoMotor.write(angle);
          delay(50);
        }
        servoObert = false;
        Serial.println("Servo tancat.");
      } else {
        Serial.println("El servo ja està tancat.");
      }
    } else if (ordre == "llums") {
      llumsEnceses = !llumsEnceses;
      digitalWrite(llumsPin, llumsEnceses ? HIGH : LOW);
      Serial.println(llumsEnceses ? "Llums enceses." : "Llums apagades.");
    } else if (ordre == "rega") {
      bombaEnc = !bombaEnc;
      digitalWrite(bombaPin, bombaEnc ? HIGH : LOW);
      Serial.println(bombaEnc ? "Bomba encesa." : "Bomba apagada.");
    } else {
      Serial.println("Ordre no reconeguda.");
    }
  }
}

bool inicialitzaAHT10() {
  if (aht10.begin()) {
    Serial.println("AHT10 inicialitzat correctament.");
    return true;
  } else {
    Serial.println("Error: No s'ha pogut inicialitzar l'AHT10.");
    return false;
  }
}

bool inicialitzaVEML7700() {
  if (veml7700.begin()) {
    Serial.println("VEML7700 inicialitzat correctament.");
    return true;
  } else {
    Serial.println("Error: No s'ha pogut inicialitzar el VEML7700.");
    return false;
  }
}
