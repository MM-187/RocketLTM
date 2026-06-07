/*
  RocketLTM RX - ESP-NOW receptor con visualizacion de datos
  Recibe la estructura enviada por el TX y muestra GPS + MPU6050 + BMP280
  en el Monitor Serial.
*/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ============================================================
// ESTRUCTURA DE DATOS - DEBE COINCIDIR EXACTAMENTE CON TX
// ============================================================
typedef struct struct_message {
  uint32_t packetId;

  // GPS
  uint32_t gpsChars;
  bool gpsLocationValid;
  double gpsLat;
  double gpsLng;
  bool gpsAltitudeValid;
  double gpsAltitude;
  int gpsSatellites;
  double gpsSpeedKmph;

  // MPU6050
  bool mpuOk;
  float accX;
  float accY;
  float accZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float tempMpu;

  // BMP280
  bool bmpOk;
  float tempBmp;
  float tempBmpCal;
  float pressureHpa;
  float altitudeBmp;
} struct_message;

struct_message message;

uint32_t packetsReceived = 0;
uint32_t lastPacketId = 0;
uint32_t lostPackets = 0;
unsigned long lastPacketMillis = 0;

// ============================================================
// FUNCION PARA IMPRIMIR MAC DEL EMISOR
// ============================================================
void printMac(const uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
}

// ============================================================
// FUNCION PARA VISUALIZAR LOS DATOS RECIBIDOS
// ============================================================
void printReceivedData(const uint8_t *mac, int len) {
  Serial.println();
  Serial.println("==================================================");
  Serial.println("           ROCKETLTM RX - DATOS RECIBIDOS");
  Serial.println("==================================================");

  Serial.print("MAC del TX: ");
  printMac(mac);
  Serial.println();

  Serial.print("Bytes recibidos: ");
  Serial.println(len);

  Serial.print("ID paquete: ");
  Serial.println(message.packetId);

  Serial.print("Paquetes recibidos: ");
  Serial.println(packetsReceived);

  Serial.print("Paquetes perdidos estimados: ");
  Serial.println(lostPackets);

  Serial.println("--------------------------------------------------");
  Serial.println("GPS NEO-6M");

  Serial.print("Caracteres recibidos GPS: ");
  Serial.println(message.gpsChars);

  Serial.print("Satelites: ");
  Serial.println(message.gpsSatellites);

  Serial.print("Velocidad: ");
  Serial.print(message.gpsSpeedKmph);
  Serial.println(" km/h");

  if (message.gpsLocationValid) {
    Serial.print("Latitud: ");
    Serial.println(message.gpsLat, 6);

    Serial.print("Longitud: ");
    Serial.println(message.gpsLng, 6);
  } else {
    Serial.println("Ubicacion: sin fix GPS todavia");
  }

  if (message.gpsAltitudeValid) {
    Serial.print("Altitud GPS: ");
    Serial.print(message.gpsAltitude);
    Serial.println(" m");
  } else {
    Serial.println("Altitud GPS: no disponible");
  }

  Serial.println("--------------------------------------------------");
  Serial.println("MPU6050");

  if (message.mpuOk) {
    Serial.print("Aceleracion X: ");
    Serial.print(message.accX);
    Serial.println(" m/s2");

    Serial.print("Aceleracion Y: ");
    Serial.print(message.accY);
    Serial.println(" m/s2");

    Serial.print("Aceleracion Z: ");
    Serial.print(message.accZ);
    Serial.println(" m/s2");

    Serial.print("Giroscopio X: ");
    Serial.print(message.gyroX);
    Serial.println(" rad/s");

    Serial.print("Giroscopio Y: ");
    Serial.print(message.gyroY);
    Serial.println(" rad/s");

    Serial.print("Giroscopio Z: ");
    Serial.print(message.gyroZ);
    Serial.println(" rad/s");

    Serial.print("Temperatura interna MPU: ");
    Serial.print(message.tempMpu);
    Serial.println(" C");
  } else {
    Serial.println("Estado: MPU6050 no detectado en TX");
  }

  Serial.println("--------------------------------------------------");
  Serial.println("BMP280");

  if (message.bmpOk) {
    Serial.print("Temperatura BMP real: ");
    Serial.print(message.tempBmp);
    Serial.println(" C");

    Serial.print("Temperatura BMP calibrada: ");
    Serial.print(message.tempBmpCal);
    Serial.println(" C");

    Serial.print("Presion: ");
    Serial.print(message.pressureHpa);
    Serial.println(" hPa");

    Serial.print("Altitud BMP aproximada: ");
    Serial.print(message.altitudeBmp);
    Serial.println(" m");
  } else {
    Serial.println("Estado: BMP280 no detectado en TX");
  }

  Serial.println("==================================================");
}

// ============================================================
// CALLBACK DE RECEPCION ESP-NOW
// ============================================================
void onDataReceived(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(message)) {
    Serial.println();
    Serial.println("Paquete recibido, pero el tamano no coincide.");
    Serial.print("Bytes recibidos: ");
    Serial.println(len);
    Serial.print("Bytes esperados: ");
    Serial.println(sizeof(message));
    return;
  }

  memcpy(&message, incomingData, sizeof(message));

  packetsReceived++;
  lastPacketMillis = millis();

  if (lastPacketId != 0 && message.packetId > lastPacketId + 1) {
    lostPackets += (message.packetId - lastPacketId - 1);
  }

  lastPacketId = message.packetId;

  printReceivedData(mac, len);
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==================================================");
  Serial.println("RocketLTM RX - Receptor ESP-NOW");
  Serial.println("==================================================");

  WiFi.mode(WIFI_STA);

  Serial.print("MAC del receptor: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataReceived);

  Serial.println("Receptor listo. Esperando datos del TX...");
  Serial.println("==================================================");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  static unsigned long lastStatusMillis = 0;

  if (millis() - lastStatusMillis >= 5000) {
    lastStatusMillis = millis();

    if (packetsReceived == 0) {
      Serial.println("Esperando paquetes ESP-NOW...");
    } else {
      Serial.print("RX activo. Ultimo paquete recibido hace ");
      Serial.print((millis() - lastPacketMillis) / 1000);
      Serial.println(" s.");
    }
  }
}
