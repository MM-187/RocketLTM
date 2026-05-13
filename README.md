# Proyecto GPS ESP32 con GeoLinker

Proyecto desarrollado con ESP32 utilizando PlatformIO para adquisición y transmisión de datos GPS.

## Descripción

El sistema obtiene datos de ubicación desde un módulo GPS NEO-6M conectado al ESP32 y permite:

- Lectura de coordenadas GPS
- Visualización de datos en pantalla OLED
- Envío de información a la plataforma GeoLinker mediante WiFi
- Almacenamiento temporal offline cuando no hay conexión
- Reconexión automática a la red

## Funcionalidades implementadas

- Latitud y longitud
- Altitud
- Velocidad
- Hora UTC
- Número de satélites
- Conexión WiFi
- Envío de datos a la nube
- Buffer offline
- Manejo de errores y estados

## Hardware utilizado

- ESP32 DevKit
- Módulo GPS NEO-6M
- Pantalla OLED SSD1306

## Librerías utilizadas

- TinyGPSPlus
- Adafruit SSD1306
- Adafruit GFX Library
- ArduinoJson

## Configuración de pines

### GPS
- RX -> GPIO 16
- TX -> GPIO 17

### OLED
- SDA -> GPIO 21
- SCL -> GPIO 22

## Archivos principales

- `src/main.cpp`
  - Visualización de datos GPS en pantalla OLED

- `src/main1.cpp`
  - Implementación de GeoLinker y transmisión de datos por WiFi

- `include/gps_config.h`
  - Configuración de pines y velocidades seriales

- `lib/GeoLinker/src/GeoLinker.cpp`
  - Implementación de la librería GeoLinker

- `lib/GeoLinker/src/GeoLinker.h`
  - Definiciones y clases de GeoLinker

- `platformio.ini`
  - Configuración del entorno PlatformIO y dependencias

## Entorno de desarrollo

- Visual Studio Code
- PlatformIO
- Framework Arduino para ESP32