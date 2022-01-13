#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


void muestreo(void *pvParameters);
void ventana(void *pvParameters);
void clasificacion(void *pvParameters);

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

//RTOS
int periodoInicio = 10;
int prioridadInicio = 4;

int periodoMuestreo = 10;
int prioridadMuestreo = 3;

int ventanaMedidas = 300;
int periodoVentana = 1500;
int prioridadVentana = 2;

int periodoClasificacion = 1500;
int prioridadClasificacion = 1;

int valorInicial, valorFinal;

float aX[300];
float aY[300];
float aZ[300];
float gX[300];
float gY[300];
float gZ[300];

int boton = 0;
float auxAx, auxAy, auxAz, auxGx, auxGy, auxGz;
//float auxAx2, auxAy2, auxAz2, auxGx2, auxGy2, auxGz2;
float mediaAX, mediaAY, mediaAZ, mediaGX, mediaGY, mediaGZ, mediaA, mediaG;
//float vEficazAX, vEficazAY, vEficazAZ, vEficazGX, vEficazGY, vEficazGZ, vEficazA, vEficazG;
//float diffAx, diffAy, diffAz, diffGx, diffGy, diffGz;
//float auxDesvAx, auxDesvAy, auxDesvAz, auxDesvGx, auxDesvGy, auxDesvGz;
//float desvAX, desvAY, desvAZ, desvGX, desvGY, desvGZ;
//float auxAx, auxAy, auxAz;
float auxAx2, auxAy2, auxAz2, auxGx2, auxGy2;
//float mediaAX, mediaAY, mediaAZ, mediaA;
float vEficazAX, vEficazAY, vEficazAZ, vEficazGX, vEficazGY, vEficazGZ, vEficazA, vEficazG;
float diffGx, diffGz;
float auxDesvGx, auxDesvGz;
float desvGX, desvGZ;


void printLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Hora no obtenida");
    return;
  }
}

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

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  sensor.setWire(&Wire);
  sensor.beginAccel();
  sensor.beginGyro();
  Serial.begin(115200);

  //pinMode(12, OUTPUT);
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

  //  client.enableDebuggingMessages();
  //  client.enableLastWillMessage("Sensor/lastwill", "I am going offline");

  xTaskCreatePinnedToCore(inicio, "Inicio", 1024, NULL, prioridadInicio, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(muestreo, "Muestreo", 7200, NULL, prioridadMuestreo, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(ventana, "Ventana", 72000, NULL, prioridadVentana, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(clasificacion, "Clasificacion", 1024, NULL, prioridadClasificacion, NULL, ARDUINO_RUNNING_CORE);

}

void loop() {
}

void inicio(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    //Serial.println(boton);
    if (digitalRead(14) == LOW && boton == 0) {
      boton = 1;
      printLocalTime();
      Serial.println(boton);
    }
    //    else if (digitalRead(14) == LOW && boton == 1) {
    //      boton = 0;
    //      mandaFichero();
    //    }
    vTaskDelay (periodoInicio);
  }
}


int i = 0;
int aux = 0;
int aux2 = 0;
void muestreo(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    sensor.accelUpdate();
    sensor.gyroUpdate();
    if (boton == 1) {
      i++;
      aX[i] = sensor.accelX();
      aY[i] = sensor.accelY();
      aZ[i] = sensor.accelZ();
      gX[i] = sensor.gyroX();
      gY[i] = sensor.gyroY();
      gZ[i] = sensor.gyroZ();
      if (i == 2 * ventanaMedidas) {
        i = 0;
      }
    }
    vTaskDelay (periodoMuestreo);
  }
}
void ventana(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (boton == 1) {
      if (aZ[i] == 10 || aZ[i] == -10) {
        if ( i >= 75) {
          valorInicial = i - 75;
          valorFinal = i + 75;
        }
        else {
          valorInicial = 300 + i - 75;
          valorFinal = i + 75;
        }
      }
      for (int x = 0; x < 150; x ++) {
        aux = valorInicial + x;
        if (aux >= 300) {
          aux = aux - 300;
        }
        auxAx = auxAx + aX[aux];
        auxAy = auxAy + aY[aux];
        auxAz = auxAz + aZ[aux];
        //auxGx = auxGx + gX[aux];
        //auxGy = auxGy + gY[aux];
        //auxGz = auxGz + gZ[aux];

        auxAx2 = auxAx2 + aX[aux] * aX[aux];
        auxAy2 = auxAy2 + aY[aux] * aY[aux];
        auxAz2 = auxAz2 + aZ[aux] * aZ[aux];
        auxGx2 = auxGx2 + gX[aux] * gX[aux];
        auxGy2 = auxGy2 + gY[aux] * gY[aux];
        //auxGz2 = auxGz2 + gZ[aux] * gZ[aux];
      }
      mediaAX = auxAx / 150;
      mediaAY = auxAy / 150;
      mediaAZ = auxAx / 150;
      mediaA = sqrt(mediaAX * mediaAX + mediaAY * mediaAY + mediaAZ * mediaAZ);
      mediaGX = auxGx / 150;
      //mediaGY = auxGy / 150;
      mediaGZ = auxGz / 150;
      //mediaG = sqrt(mediaGX * mediaGX + mediaGY * mediaGX + mediaGZ * mediaGZ);

      vEficazAX = sqrt(auxAx2 / 150);
      vEficazAY = sqrt(auxAy2 / 150);
      vEficazAZ = sqrt(auxAx2 / 150);
      vEficazA = sqrt(vEficazAX * vEficazAX + vEficazAY * vEficazAY + vEficazAZ * vEficazAZ);
      vEficazGX = sqrt(auxGx2 / 150);
      vEficazGY = sqrt(auxGy2 / 150);
      //vEficazGZ = sqrt(auxGz2 / 150);
      //vEficazG = sqrt(vEficazGX * vEficazGX + vEficazGY * vEficazGY + vEficazGZ * vEficazGZ);

      for (int y = 0; y < 150; y ++) {
        aux2 = valorInicial + y;
        if (aux2 >= 300) {
          aux2 = aux2 - 300;
        }
        //diffAx = aX[aux2] - mediaAX;
        //diffAy = aY[aux2] - mediaAY;
        //diffAz = aZ[aux2] - mediaAZ;
        diffGx = gX[aux2] - mediaGX;
        //diffGy = gY[aux2] - mediaGY;
        diffGz = gZ[aux2] - mediaGZ;
        //auxDesvAx = auxDesvAx + diffAx * diffAx;
        //auxDesvAy = auxDesvAy + diffAy * diffAy;
        //auxDesvAz = auxDesvAz + diffAz * diffAz;
        auxDesvGx = auxDesvGx + diffGx * diffGx;
        //auxDesvGy = auxDesvGy + diffGy * diffGy;
        auxDesvGz = auxDesvGz + diffGz * diffGz;
      }
      //desvAX = sqrt(auxDesvAx / 149);
      //desvAY = sqrt(auxDesvAy / 149);
      //desvAZ = sqrt(auxDesvAz / 149);
      desvGX = sqrt(auxDesvGx / 149);
      //desvGY = sqrt(auxDesvGy / 149);
      desvGZ = sqrt(auxDesvGz / 149);
    }
    vTaskDelay (periodoVentana);
  }
}

void clasificacion(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (boton == 1) {
      Serial.println(mediaA);
      Serial.println(vEficazA);
      Serial.println(vEficazGX);
      Serial.println(vEficazGY);
      if (aZ[i] == 10 || aZ[i] == -10) {
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
          movimiento = "Movimiento inconcluyente";
          Serial.println("Movimiento inconcluyente");
        }
        almacenaDatos();
        ///ENVIO LA INFORMACIÓN AL MOVIL
        //client.publish("Sensor", movimiento);
        ///ENVIO LA INFORMACIÓN AL MOVIL
      }
    }
    vTaskDelay (periodoClasificacion);
  }
}
