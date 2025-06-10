//Variables para conectarse con Blynk
#define BLYNK_TEMPLATE_ID "TMPL27CJAET4n"
#define BLYNK_TEMPLATE_NAME "Sistema de Riego Inteligente"
#define BLYNK_AUTH_TOKEN "YNdTFGrRzLuzupayV_QwZWq09mMPkFAL"

#include "DHT.h" 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>   // I2C
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define DHTPIN       23
#define DHTTYPE      DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


//BlynkTimer timer;

char network[] = "Fibertel WiFi804 2.4GHz";
char password[] = "00437475146";

const int evPin     = 15;     // Pin de la electroválvula
float tiempoEv      = 10.0;   // Duración de apertura en segundos

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables para manejar riego con millis()
bool   riegoActivo   = false;
unsigned long inicioRiego = 0;
unsigned long tiempoUltimoRiego =0;
const unsigned long intervaloRiego = 12UL * 60UL * 60UL * 1000UL; //12 horas en milisegundos


void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN,network,password);
  dht.begin();
  
  // Pines
  pinMode(evPin, OUTPUT);
  digitalWrite(evPin, LOW);

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encuentra el OLED!"));
    while(true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0,10);
  display.println("Pantalla  OK!");
  display.display();
}

void loop() {
  Blynk.run();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Leo el sensor cada 10 segundos
  static unsigned long lastRead = 0;
  unsigned long ahora = millis(); //para guardar el valor actual de tiempo

  if (millis() - lastRead >= 10000) {
    lastRead = millis();
    leerYMostrarSensor();
    
    // Si humedad < 40% o temp > 30°C, y pasaron 12 horas del ultimo riego, arranca riego
    bool debeRegar = (h < 40.0 || t > 16.9); // podés ajustar condiciones
    if (!riegoActivo && debeRegar && ((ahora - tiempoUltimoRiego) >= intervaloRiego)) {
    iniciarRiego();
    tiempoUltimoRiego = ahora;
    }
  }

  // Si el riego está activo, comprobamos el tiempo transcurrido
  if (riegoActivo && millis() - inicioRiego >= (unsigned long)(tiempoEv * 1000)) {
    finalizarRiego();
  }

  // Aquí podrías atender comandos de Blynk u otras tareas  
}

// Lee DHT y actualiza OLED
void leerYMostrarSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Fallo lectura DHT!"));
    return;
  }
  Serial.printf("Temp: %.1f C, Hum: %.1f %%\n", t, h);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.printf("T:%.1fC\nH:%.1f%%", t, h);
  display.display();

  Blynk.virtualWrite(V0,h);
  Blynk.virtualWrite(V1,t);
}

// Arranca el riego
void iniciarRiego() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.printf("Iniciando riego...");
  display.display();
  digitalWrite(evPin, HIGH);
  inicioRiego = millis();
  riegoActivo = true;
}

// Finaliza el riego
void finalizarRiego() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.printf("Riego     finalizado.");
  display.display();
  digitalWrite(evPin, LOW);
  riegoActivo = false;
}
