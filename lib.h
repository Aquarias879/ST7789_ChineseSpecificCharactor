#ifndef lib_h
#define lib_h

#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>
#include <WiFi.h> 
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include "Fonts/Yu_hei.h"
#include "tftIcons/img.h"
#include "MAX30105.h"
#include "StepDetector.h"
#include "spo2_algorithm.h"

//XIAO ESP32C3 Wiring  DIN 10 CLK 8 BL 9 CS 5 RST 4 DC 3
#define TFT_CS   D1
#define TFT_RST  D3 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC   D2
#define ADC_PIN  A0
#define TFT_BL   D9

#define TFT_HEIGHT 280
#define TFT_WIDTH  240

const char* serverAddress = "192.168.2.140";
const int serverPort = 8000;

TFT_eSPI tft = TFT_eSPI();
StepDetector stepDetector;
MAX30105 particleSensor;
WiFiManager wm;
WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, serverAddress, serverPort);

uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

int8_t validSpO2Count = 0;
int32_t totalSpO2 = 0;
int32_t totalHr = 0;
int32_t averageSpO2 = 0;
int32_t averageHR = 0;

float vec_val;
float calibration = -22.15;
int bat_percentage,sensorValue,deepsleep,lightsleep,totallight,totaldeep,totalsleep,walking;
bool dis_state;

unsigned long lastMeasurementTime = 0;
unsigned long lastDisplayTime = 0;
const unsigned long measurementInterval = 30 * 60 * 1000; 
const unsigned long dis_Interval = 0.5 * 60 * 1000; 

float minVoltage = 3000.0; // 3.0V
float maxVoltage = 4200.0; // 4.2V

String jsonString;

void SPO2_HR_handle() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    irBuffer[i] = particleSensor.getIR();
    redBuffer[i] = particleSensor.getRed(); 
    particleSensor.nextSample(); 
    delay(10); 
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  //delay(50);
}

String generateDeviceID(){
  // Get the ESP32's unique chip ID
  uint64_t chipId = ESP.getEfuseMac();
  // Convert the chip ID to a string
  char deviceID[17]; // 64-bit ID = 16 characters (digits and letters) + null terminator
  sprintf(deviceID, "%04X%08X", (uint16_t)(chipId >> 32), (uint32_t)chipId);
  return String(deviceID);

}

UBaseType_t getStackHighWaterMark() {
    return uxTaskGetStackHighWaterMark(NULL);
}

void font16(int x, int y, String text, uint16_t color) {
  tft.loadFont(Microsoft_Yahei16);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.print(text);
}

void font20(int x, int y, String text, uint16_t color) {
  tft.loadFont(Microsoft_Yahei20);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.print(text);
}

void resetValues() {
  totalSpO2 = 0;
  totalHr = 0;
  validSpO2Count = 0;
  walking = 0;
  // Reset any other values you want to reset to zero here
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float Battery() {
  sensorValue = analogRead(ADC_PIN);
  float voltage = (((sensorValue * 3.3) / 1024) * 2 + calibration); //multiply by two as voltage divider network is 100K & 100K Resistor

  bat_percentage = mapfloat(voltage, 2.8, 3.8, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage

  if (bat_percentage >= 100) {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0) {
    bat_percentage = 1;
  }

  return bat_percentage;
}



#endif