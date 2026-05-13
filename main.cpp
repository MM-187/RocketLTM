#include <TinyGPSPlus.h>

#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =========================
// OLED CONFIG
// =========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =========================
// GPS CONFIG
// =========================

TinyGPSPlus gps;

#define gpsSerial Serial2

unsigned long lastDisplay = 0;
void updateDisplay();
// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // OLED I2C
  Wire.begin(21, 22);

  // Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED failed");
    while (true);
  }

  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(WHITE);

  display.setCursor(0, 0);

  display.println("GPS Starting...");

  display.display();

  delay(2000);
}

// =========================
// LOOP
// =========================

void loop() {

  // Leer datos GPS
  while (gpsSerial.available() > 0) {

    gps.encode(gpsSerial.read());
  }

  // Verificar GPS
  if (millis() > 5000 && gps.charsProcessed() < 10) {

    Serial.println("GPS not detected");

    display.clearDisplay();

    display.setCursor(0, 0);

    display.println("GPS ERROR");

    display.display();

    while (true);
  }

  // Actualizar pantalla cada segundo
  if (millis() - lastDisplay > 1000) {

    updateDisplay();

    lastDisplay = millis();
  }
}

// =========================
// FUNCIÓN OLED
// =========================

void updateDisplay() {

  display.clearDisplay();

  display.setCursor(0, 0);

  // =========================
  // FIX GPS
  // =========================

  if (gps.location.isValid()) {

    display.println("GPS FIX OK");

    display.print("Lat:");

    display.println(gps.location.lat(), 6);

    display.print("Lng:");

    display.println(gps.location.lng(), 6);

  } else {

    display.println("Waiting FIX...");
  }

  // =========================
  // SATELITES
  // =========================

  display.print("SAT:");

  display.println(gps.satellites.value());

  // =========================
  // ALTITUD
  // =========================

  display.print("ALT:");

  display.print(gps.altitude.meters());

  display.println("m");

  // =========================
  // VELOCIDAD
  // =========================

  display.print("SPD:");

  display.print(gps.speed.kmph());

  display.println("km/h");

  // =========================
  // HORA UTC
  // =========================

  if (gps.time.isValid()) {

    display.print("UTC:");

    if (gps.time.hour() < 10) display.print("0");
    display.print(gps.time.hour());

    display.print(":");

    if (gps.time.minute() < 10) display.print("0");
    display.print(gps.time.minute());

    display.print(":");

    if (gps.time.second() < 10) display.print("0");
    display.println(gps.time.second());
  }

  display.display();
}