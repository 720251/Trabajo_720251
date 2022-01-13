#include "time.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32_FTPClient.h>
#include <MPU9250_asukiaaa.h>
#include <Wire.h>

//#include "EspMQTTClient.h"

#define SDA_PIN 21
#define SCL_PIN 22

MPU9250_asukiaaa sensor;

volatile int interruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

//WIFI
const char* ssid       = "vodafoneBA1157";
const char* password   = "SRULAGD6RHFQ4M5K";
//const char* ssid       = "MiA2";
//const char* password   = "25208230t";

//EspMQTTClient client(
//  "MiA2",
//  "25208230t",
//  "test.mosquito.org",
//  "Sensor",
//  883
//  );

//MARCA TEMPORAL
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;
struct tm timeinfo;

//FTP
char ftp_server[] = "155.210.150.77";
char ftp_user[]   = "rsense";
char ftp_pass[]   = "rsense";
ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass, 5000, 2);


int valorInicial, valorFinal;

float aX[300];
float aY[300];
float aZ[300];
float A[300];
float gX[300];
float gY[300];
float gZ[300];

int boton = 0;
float auxAx, auxAy, auxAz, auxGx, auxGy, auxGz, auxA;
float auxAx2, auxAy2, auxAz2, auxGx2, auxGy2, auxGz2;
float mediaAX, mediaAY, mediaAZ, mediaGX, mediaGY, mediaGZ, mediaA, mediaG;
float vEficazAX, vEficazAY, vEficazAZ, vEficazGX, vEficazGY, vEficazGZ, vEficazA, vEficazG;
float diffAx, diffAy, diffAz, diffGx, diffGy, diffGz;
float auxDesvAx, auxDesvAy, auxDesvAz, auxDesvGx, auxDesvGy, auxDesvGz;
float desvAX, desvAY, desvAZ, desvGX, desvGY, desvGZ;

int contadorMuestreo;
int contadorClasificacion;
int periodoMuestreo = 10;
int periodoventana = 3000;
int periodoClasificacion = 1500;
unsigned long int actualmillis = 0;
unsigned long int ventanamillis = 0;

int i = 0;
int aux = 0;
int aux2 = 0;
int ventanaIniciada = 0;

String nombreString, infoString, parameter;
String movimiento;
int numRegistros = 0;
char nombreChar[25];
char infoChar[75000];

void almacenaDatos() {

  //ftp.InitFile("Type A");
  nombreString = String(timeinfo.tm_mon) + String(timeinfo.tm_mday) + String(timeinfo.tm_hour) + String(timeinfo.tm_min) + ".csv";
  nombreString.toCharArray(nombreChar, 20);
  parameter = String(String(movimiento) + ";" + String(timeinfo.tm_mon) + ";" + String(timeinfo.tm_mday) + ";" +
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

void muestreo() {
  sensor.accelUpdate();
  sensor.gyroUpdate();
  i++;
  aX[i] = sensor.accelX();
  aY[i] = sensor.accelY();
  aZ[i] = sensor.accelZ();
  gX[i] = sensor.gyroX();
  gY[i] = sensor.gyroY();
  gZ[i] = sensor.gyroZ();
  if (i == 300) {
    i = 0;
  }
}
void ventana() {
  if ((aZ[i] == 4.0 || aZ[i] == -4.0) && ventanaIniciada == 0) {
    int j = i;
    ventanaIniciada = 1;
    ventanamillis = millis();
    if (j >= 75) {
      valorInicial = j - 75;
      valorFinal = j + 75;
    }
    else {
      valorInicial = 300 + j - 75;
      valorFinal = j + 75;
    }
  }
}

void analisis() {
  for (int x = 0; x < 150; x ++) {
    aux = valorInicial + x;
    if (aux >= 300) {
      aux = aux - 300;
    }
    auxAx = auxAx + aX[aux];
    auxAy = auxAy + aY[aux];
    auxAz = auxAz + aZ[aux];
    auxGx = auxGx + gX[aux];
    auxGy = auxGy + gY[aux];
    auxGz = auxGz + gZ[aux];

    auxAx2 = auxAx2 + aX[aux] * aX[aux];
    auxAy2 = auxAy2 + aY[aux] * aY[aux];
    auxAz2 = auxAz2 + aZ[aux] * aZ[aux];
    auxGx2 = auxGx2 + gX[aux] * gX[aux];
    auxGy2 = auxGy2 + gY[aux] * gY[aux];
    auxGz2 = auxGz2 + gZ[aux] * gZ[aux];
  }
  mediaAX = auxAx / 150;
  mediaAY = auxAy / 150;
  mediaAZ = auxAx / 150;
  mediaA = sqrt(mediaAX * mediaAX + mediaAY * mediaAY + mediaAZ * mediaAZ);
  mediaGX = auxGx / 150;
  mediaGY = auxGy / 150;
  mediaGZ = auxGz / 150;
  mediaG = sqrt(mediaGX * mediaGX + mediaGY * mediaGX + mediaGZ * mediaGZ);

  vEficazAX = sqrt(auxAx2 / 150);
  vEficazAY = sqrt(auxAy2 / 150);
  vEficazAZ = sqrt(auxAx2 / 150);
  vEficazA = sqrt(vEficazAX * vEficazAX + vEficazAY * vEficazAY + vEficazAZ * vEficazAZ);
  vEficazGX = sqrt(auxGx2 / 150);
  vEficazGY = sqrt(auxGy2 / 150);
  vEficazGZ = sqrt(auxGz2 / 150);
  vEficazG = sqrt(vEficazGX * vEficazGX + vEficazGY * vEficazGY + vEficazGZ * vEficazGZ);

  for (int y = 0; y < 150; y ++) {
    aux2 = valorInicial + y;
    if (aux2 >= 300) {
      aux2 = aux2 - 300;
    }
    diffAx = aX[aux2] - mediaAX;
    diffAy = aY[aux2] - mediaAY;
    diffAz = aZ[aux2] - mediaAZ;
    diffGx = gX[aux2] - mediaGX;
    diffGy = gY[aux2] - mediaGY;
    diffGz = gZ[aux2] - mediaGZ;
    auxDesvAx = auxDesvAx + diffAx * diffAx;
    auxDesvAy = auxDesvAy + diffAy * diffAy;
    auxDesvAz = auxDesvAz + diffAz * diffAz;
    auxDesvGx = auxDesvGx + diffGx * diffGx;
    auxDesvGy = auxDesvGy + diffGy * diffGy;
    auxDesvGz = auxDesvGz + diffGz * diffGz;
  }
  desvAX = sqrt(auxDesvAx / 149);
  desvAY = sqrt(auxDesvAy / 149);
  desvAZ = sqrt(auxDesvAz / 149);
  desvGX = sqrt(auxDesvGx / 149);
  desvGY = sqrt(auxDesvGy / 149);
  desvGZ = sqrt(auxDesvGz / 149);


  auxAx = 0;  auxAy = 0;  auxAz = 0;  auxGx = 0;  auxGy = 0;  auxGz = 0;  auxA = 0;
  auxAx2 = 0;  auxAy2 = 0;  auxAz2 = 0;  auxGx2 = 0;  auxGy2 = 0;  auxGz2 = 0;
  diffAx = 0;  diffAy = 0;  diffAz = 0;  diffGx = 0;  diffGy = 0;  diffGz = 0;
  auxDesvAx = 0;  auxDesvAy = 0;  auxDesvAz = 0;  auxDesvGx = 0;  auxDesvGy = 0;  auxDesvGz = 0;

}

void clasificacion() {
  Serial.println(mediaA);
  Serial.println(vEficazA);
  Serial.println(vEficazGX);
  Serial.println(vEficazGY);
  if ((mediaA > 0.95) && (mediaA < 1.95) && (vEficazA > 2.05) && (vEficazA < 3.8)) {
    //PASE
    if ((vEficazGX > 150.0) && (vEficazGY > 160.0)) {
      movimiento = "Pase exterior";
      Serial.println("Pase exterior");
    }
    else if ((vEficazGX > 95.0) && (vEficazGX < 150.0) && (vEficazGY > 105.0) && (vEficazGY < 160.0)) {
      movimiento = "Pase interior";
      Serial.println("Pase interior");
    }
    else {
      movimiento = "Pase inconcluyente";
      Serial.println("Pase inconcluyente");
    }
  }
  else if ((mediaA > 1.95) && (vEficazA > 3.8)) {
    //CHUTE
    if ((desvGX > 225.0) && (desvGZ > 250.0)) {
      movimiento = "Chute empeine";
      Serial.println("Chute empeine");
    }
    else if ((desvGX < 225.0) && (desvGZ < 250.0)) {
      movimiento = "Chute puntera";
      Serial.println("Chute puntera");
    }
    else {
      movimiento = "Chute inconcluyente";
      Serial.println("Chute inconcluyente");
    }
  }
  else {
    //movimiento = "Movimiento inconcluyente";
    Serial.println("Movimiento inconcluyente");
  }
  //almacenaDatos();
  ///ENVIO LA INFORMACIÓN AL MOVIL
  //client.publish("Sensor", movimiento);
  ///ENVIO LA INFORMACIÓN AL MOVIL
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
  WiFi.setSleep(false); // Desactiva la suspensión de wifi en modo STA para mejorar la velocidad de respuesta
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

  //  client.enableDebuggingMessages();
  //  client.enableLastWillMessage("Sensor/lastwill", "I am going offline");

}

void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Hora no obtenida");
    return;
  }
}

void loop() {
  printLocalTime();
  if (interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    if (digitalRead(14) == LOW && boton == 0) {
      boton = 1;
      Serial.println(boton);
    }
    if (boton == 1) {
      contadorMuestreo++;
      contadorClasificacion++;
      if (contadorMuestreo == periodoMuestreo) {
        muestreo();
        contadorMuestreo = 0;
      }
      //Serial.println(aZ[i]);
      ventana();
      if (actualmillis - ventanamillis <= periodoventana) {
        analisis();
        if(contadorClasificacion == periodoClasificacion) {
          clasificacion();
          contadorClasificacion = 0;
        }
        
      }
      if (actualmillis - ventanamillis > periodoventana) {
        actualmillis = millis();
        ventanaIniciada = 0;
      }
    }
  }
}
