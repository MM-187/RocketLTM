# RocketLTM Modulo de sensado

Proyecto desarrollado con ESP32 utilizando PlatformIO para adquisición y transmisión de datos GPS.

## Descripción

El sistema obtiene datos de los modulos GPS NEO-6M, MPU 6050 y BMP 280 conectados al ESP32 y permite:

- Leer datos de el ambiente como:
- Latitud
- Longitud
- Altitud
- Aceleracion
- Presion barometrica
- Temperatura

- Transmitir esos datos a travez de el protocolo ESP-NOW
- Mostrar los datos recibidos en pantalla OLED y serial


## Hardware utilizado

- ESP32 DevKit
- Módulo GPS NEO-6M
- Pantalla OLED SSD1306
- Módulo MPU 6050
- Módulo BMP 280

## Librerías utilizadas

- TinyGPSPlus
- Adafruit SSD1306
- Adafruit MPU6050
- Adafruit BMP280
- Adafruit GFX Library
- ArduinoJson

## Configuración de pines

### GPS
- RX -> GPIO 16
- TX -> GPIO 17

### OLED, MPU y BMP
- SDA -> GPIO 21
- SCL -> GPIO 22

## Archivos principales

- `tx/src/main.cpp`
  - Sensado y Transmision de datos a travez de ESP-NOW
    
- `rx/src/main.cpp`
  - Recepcion y Visualización de datos en pantalla OLED

- `rx/platformio.ini`
  - Configuración del entorno PlatformIO y dependencias

- `tx/platformio.ini`
  - Configuración del entorno PlatformIO y dependencias

## Entorno de desarrollo

- Visual Studio Code
- PlatformIO
- Framework Arduino para ESP32
