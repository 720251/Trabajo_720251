#include "time.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32_FTPClient.h>
#include <MPU9250_asukiaaa.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

MPU9250_asukiaaa sensor;

volatile int interruptCounter;
int totalInterruptCounter;
int contadorSerial;
int contadorLed;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

//WIFI
//const char* ssid       = "vodafoneBA1157";
//const char* password   = "SRULAGD6RHFQ4M5K";
const char* ssid       = "MiA2";
const char* password   = "25208230t";

//MARCA TEMPORAL
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;
struct tm timeinfo;

void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Hora no obtenida");
    return;
  }
}
 //VARIABLES PARA MILLIS
int boton = 0;
unsigned long int actualmillis = 0;
unsigned long int botonmillis = 0;
unsigned long int tiempoMuestreo = 14000;
unsigned long int tiempoActivacion = 27000;


//FTP
char ftp_server[] = "155.210.150.77";
char ftp_user[]   = "rsense";
char ftp_pass[]   = "rsense";
ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass, 5000, 2);

String nombreString, infoString, parameter;
int numRegistros = 0;
char nombreChar[25];
char infoChar[75000];


void almacenaDatos() {

  //ftp.InitFile("Type A");
  nombreString = String(timeinfo.tm_mon) + String(timeinfo.tm_mday) + String(timeinfo.tm_hour) + String(timeinfo.tm_min) + ".csv";
  nombreString.toCharArray(nombreChar, 20);
  parameter = String(String(sensor.accelX()) + ";" + String(sensor.accelY()) +  ";" + String(sensor.accelZ()) +  ";" + String(sensor.gyroX()) +
                     ";" + String(sensor.gyroY()) +  ";" + String(sensor.gyroZ()) + ";" + String(timeinfo.tm_mon) + ";" + String(timeinfo.tm_mday) + ";" +
                     String(timeinfo.tm_hour) + ";" + String(timeinfo.tm_min) + ";" + String(timeinfo.tm_sec) + "\n" );
  infoString.concat(parameter);
  infoString.toCharArray(infoChar, 75000);
}


void mandaFichero() {
  ftp.OpenConnection();
  ftp.ChangeWorkDir("/rsense/720251_trabajo");
  ftp.InitFile("Type A");
  ftp.NewFile(nombreChar);
  ftp.Write(infoChar);
  ftp.CloseFile();
  ftp.CloseConnection();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  sensor.setWire(&Wire);
  sensor.beginAccel();
  sensor.beginGyro();
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000, true);
  timerAlarmEnable(timer);
  pinMode(12, OUTPUT);
  pinMode(14, INPUT);
  //INICIALIZAMOS EL WIFI
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());
  //CONFIGURAMOS LA FECHA Y LA HORA
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void leeSensor() {
  if (interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    if (digitalRead(14) == LOW) {
      boton = 1;
      botonmillis = millis();
    }
    if (actualmillis - botonmillis <= tiempoActivacion && boton == 1) {
      totalInterruptCounter++;
      contadorSerial++;
      contadorLed++;
      if (totalInterruptCounter == 10) {
        sensor.accelUpdate();
        sensor.gyroUpdate();
        almacenaDatos();
        if (digitalRead(12) == LOW) {
          digitalWrite(12, HIGH);
        }
        totalInterruptCounter = 0;
      }
      if (contadorSerial == tiempoMuestreo) {
        Serial.print("Fichero: ");
        Serial.println(infoChar);
        mandaFichero();
        contadorSerial = 0;
      }
      if (digitalRead(12) == HIGH && contadorLed == 15) {
        digitalWrite(12, LOW);
        contadorLed = 0;
      }
    }
    if (actualmillis - botonmillis > tiempoActivacion && boton == 1) {
      totalInterruptCounter = 0;
      contadorSerial = 0;
      digitalWrite(12, LOW);
      contadorLed = 0;
      actualmillis = millis();
      boton = 0;

    }
  }
}

void loop() {
  actualmillis = millis();
  printLocalTime();
  leeSensor();

}
