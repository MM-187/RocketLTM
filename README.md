# Proyecto sensor DHT + MPU en ESP32

Proyecto desarrollado con ESP32 utilizando PlatformIO.

## Descripción

Se obtiene aceleracion (del sensor MPU 6050) y temperatura (de un DHTxx)
Se muestra estos datos en una pantalla OLED y por interfaz serial.

## Funcionalidades implementadas

- Temperatura
- Aceleracion

## Hardware utilizado

- ESP32 DevKit
- DHTxx
- MPU 6050

## Librerías utilizadas

- DHT sensor library
- Adafruit SSD1306
- Adafruit MPU6050

## Configuración de pines

### DHT
- DHT_PIN -> GPIO 19

### OLED y MPU 6050
- SDA -> GPIO 21
- SCL -> GPIO 22

## Entorno de desarrollo

- Visual Studio Code
- PlatformIO
- Framework Arduino para ESP32
