#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 64 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_MPU6050 mpu;  // Crea el objeto del sensor
float accel[3];


void getAccMPU(float* acc){
  sensors_event_t a, g, temp;  // a = aceleración, g = giroscopio, temp = temperatura
  mpu.getEvent(&a, &g, &temp);  // Lee todos los datos del sensor
  acc[0]=a.acceleration.x;
  acc[1]=a.acceleration.y;
  acc[2]=a.acceleration.z;
}

void showAccSerial(float* acc){
  Serial.print("Aceleracion m/s² x: " + String(acc[0],2));
  Serial.print(                " y: " + String(acc[1],2));
  Serial.println(              " z: " + String(acc[2],2));
}

void printOLED(String str){
  int16_t xCursor = display.getCursorX();
  int16_t yCursor = display.getCursorY();
  for (char i : str) { // clear writing space with ' '
    if(i=='\n') display.write(i);
    else display.write(' ');
  }
  display.setCursor(xCursor,yCursor); // reset cursor
  for (char i : str) { // actually print
    display.write(i);
  }

}

void printOLEDln(String str){printOLED(str+"\n");}

void printOLEDxy(String str, int16_t x, int16_t y){
  display.setCursor(x,y);
  printOLED(str);
}

void printOLEDxyln(String str, int16_t x, int16_t y){printOLEDxy(str+"\n",x,y);}


void showAccOLED(float* acc){
  printOLEDxyln("X "+String(acc[0],1),0,8);
  printOLEDln(  "Y "+String(acc[1],1));
  printOLEDln(  "Z "+String(acc[2],1));
  display.display();
}

void setup() {
  Serial.begin(9600);

  while(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println("Asignacion OLED fallo");
  }
  while(!mpu.begin()){
    Serial.println("No se encontro MPU 6050");
  }

  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Configura el rango del acelerómetro (±2g, ±4g, ±8g, ±16g)
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

  delay(100); //100 for mpu, 500 for tick
}

void loop() {
  delay(500);

  getAccMPU(accel);
  showAccSerial(accel);
  showAccOLED(accel);
}
