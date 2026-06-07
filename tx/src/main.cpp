/*
  RocketLTM TX - ESP-NOW + GPS + MPU6050 + BMP280
  Basado en el estilo del codigo ESP-NOW del profesor:
  - WiFi.mode(WIFI_STA)
  - peerInfo.channel = 0
  - sin forzar canal, sin WiFi.disconnect(), sin ifidx
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TinyGPSPlus.h>
#include <Adafruit_BMP280.h>

// ============================================================
// CONFIGURACION ESP-NOW
// ============================================================
// MAC del receptor RX: 8C:94:DF:4D:4D:BC
uint8_t RECEIVER_MAC[] = {0x8C, 0x94, 0xDF, 0x4D, 0x4D, 0xBC};

// Para probar por broadcast, descomente esta linea y comente la anterior:
// uint8_t RECEIVER_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t peerInfo;

// ============================================================
// PINES
// ============================================================
#define SDA_PIN 21
#define SCL_PIN 22

#define GPS_RX_PIN 16   // RX ESP32 <- TX GPS
#define GPS_TX_PIN 17   // TX ESP32 -> RX GPS

#define MPU_ADDR 0x68
#define BMP_ADDR 0x76

// ============================================================
// OBJETOS
// ============================================================
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
Adafruit_BMP280 bmp;

// ============================================================
// ESTRUCTURA DE DATOS - DEBE COINCIDIR CON RX
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
uint32_t packetId = 0;
bool bmpOk = false;
bool mpuOk = false;

// ============================================================
// FUNCIONES MPU6050 POR REGISTROS
// ============================================================
void writeMPU(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

int16_t readMPU16(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)2);

  if (Wire.available() < 2) return 0;

  int16_t value = Wire.read() << 8 | Wire.read();
  return value;
}

bool detectI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

bool initMPU6050() {
  if (!detectI2C(MPU_ADDR)) return false;

  writeMPU(0x6B, 0x00); // Wake up
  delay(100);
  writeMPU(0x1C, 0x00); // Accel +/-2g
  writeMPU(0x1B, 0x00); // Gyro +/-250 dps
  return true;
}

void readMPU6050() {
  if (!mpuOk) {
    message.mpuOk = false;
    return;
  }

  int16_t rawAx = readMPU16(0x3B);
  int16_t rawAy = readMPU16(0x3D);
  int16_t rawAz = readMPU16(0x3F);
  int16_t rawTemp = readMPU16(0x41);
  int16_t rawGx = readMPU16(0x43);
  int16_t rawGy = readMPU16(0x45);
  int16_t rawGz = readMPU16(0x47);

  message.mpuOk = true;
  message.accX = (rawAx / 16384.0) * 9.80665;
  message.accY = (rawAy / 16384.0) * 9.80665;
  message.accZ = (rawAz / 16384.0) * 9.80665;

  message.gyroX = (rawGx / 131.0) * DEG_TO_RAD;
  message.gyroY = (rawGy / 131.0) * DEG_TO_RAD;
  message.gyroZ = (rawGz / 131.0) * DEG_TO_RAD;

  message.tempMpu = (rawTemp / 340.0) + 36.53;
}

// ============================================================
// CALLBACK DE ENVIO
// ============================================================
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Estado de envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "exito" : "fallo");
}

// ============================================================
// GENERACION DE DATOS
// ============================================================
void readGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  message.gpsChars = gps.charsProcessed();
  message.gpsLocationValid = gps.location.isValid();
  message.gpsLat = gps.location.isValid() ? gps.location.lat() : 0.0;
  message.gpsLng = gps.location.isValid() ? gps.location.lng() : 0.0;
  message.gpsAltitudeValid = gps.altitude.isValid();
  message.gpsAltitude = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
  message.gpsSatellites = gps.satellites.isValid() ? gps.satellites.value() : 0;
  message.gpsSpeedKmph = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
}

void readBMP280() {
  if (!bmpOk) {
    message.bmpOk = false;
    return;
  }

  message.bmpOk = true;
  message.tempBmp = bmp.readTemperature();
  message.tempBmpCal = message.tempBmp - 4.0;
  message.pressureHpa = bmp.readPressure() / 100.0F;
  message.altitudeBmp = bmp.readAltitude(1013.25);
}

void generateSensorData() {
  packetId++;
  message.packetId = packetId;

  readGPS();
  readMPU6050();
  readBMP280();
}

// ============================================================
// ENVIO DE DATOS
// ============================================================
void sendData() {
  esp_err_t result = esp_now_send(
    RECEIVER_MAC,
    (uint8_t *) &message,
    sizeof(message)
  );

  if (result == ESP_OK) {
    Serial.println("Mensaje enviado");
  } else {
    Serial.print("Error al enviar mensaje. Codigo: ");
    Serial.println(result);
  }
}

void printLocalData() {
  Serial.println();
  Serial.println("==================================================");
  Serial.print("Paquete TX ID: ");
  Serial.println(message.packetId);

  Serial.println("GPS NEO-6M");
  Serial.print("Caracteres recibidos GPS: ");
  Serial.println(message.gpsChars);
  Serial.print("Satelites: ");
  Serial.println(message.gpsSatellites);
  if (message.gpsLocationValid) {
    Serial.print("Latitud: "); Serial.println(message.gpsLat, 6);
    Serial.print("Longitud: "); Serial.println(message.gpsLng, 6);
  } else {
    Serial.println("Ubicacion: sin fix GPS todavia");
  }

  Serial.println("--------------------------------------------------");
  Serial.println("MPU6050");
  if (message.mpuOk) {
    Serial.print("Aceleracion X: "); Serial.println(message.accX);
    Serial.print("Aceleracion Y: "); Serial.println(message.accY);
    Serial.print("Aceleracion Z: "); Serial.println(message.accZ);
    Serial.print("Giroscopio X: "); Serial.println(message.gyroX);
    Serial.print("Giroscopio Y: "); Serial.println(message.gyroY);
    Serial.print("Giroscopio Z: "); Serial.println(message.gyroZ);
    Serial.print("Temperatura interna MPU: "); Serial.println(message.tempMpu);
  } else {
    Serial.println("Estado: MPU6050 no detectado");
  }

  Serial.println("--------------------------------------------------");
  Serial.println("BMP280");
  if (message.bmpOk) {
    Serial.print("Temperatura BMP real: "); Serial.println(message.tempBmp);
    Serial.print("Temperatura BMP calibrada: "); Serial.println(message.tempBmpCal);
    Serial.print("Presion: "); Serial.print(message.pressureHpa); Serial.println(" hPa");
    Serial.print("Altitud BMP aproximada: "); Serial.print(message.altitudeBmp); Serial.println(" m");
  } else {
    Serial.println("Estado: BMP280 no detectado");
  }
  Serial.println("==================================================");
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nRocketLTM TX - Sensores + ESP-NOW estilo profesor");

  // Sensores
  Wire.begin(SDA_PIN, SCL_PIN);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  mpuOk = initMPU6050();
  Serial.println(mpuOk ? "MPU6050 detectado" : "MPU6050 no detectado");

  bmpOk = bmp.begin(BMP_ADDR);
  Serial.println(bmpOk ? "BMP280 detectado" : "BMP280 no detectado");

  // ESP-NOW segun codigo del profesor
  WiFi.mode(WIFI_STA);

  Serial.print("MAC del transmisor: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, RECEIVER_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al agregar el receptor");
    return;
  }

  Serial.println("Receptor agregado correctamente");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  generateSensorData();
  printLocalData();
  sendData();

  delay(2000);
}
