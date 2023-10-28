#include "lib.h"

String deviceID = generateDeviceID();
const char* devID = deviceID.c_str(); // Convert String to const char*

void setup() {
  Serial.begin(115200);
  pinMode(ADC_PIN, INPUT);   
  tft.init();
  tft.begin();
  tft.fillScreen(0x0000);
  font20(60,140,"正在連網路......" ,0xFFFF);
  wm.setConnectTimeout(50);
  wm.setSaveConnect(true);
  wm.setWiFiAutoReconnect(true);
  wm.autoConnect(devID,devID);
  tft.fillScreen(0x0000);
  font20(60,140,"網路連成功....",0xFFFF);
  delay(3000);
  Wire.begin();
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x3C);
  particleSensor.setPulseAmplitudeGreen(0x3C);
  particleSensor.enableDIETEMPRDY();
  tft.pushImage(0,20,240,280,bg);
  font20(60,90,"0 Steps",0xFFFF); //step
  font20(60,130,"0 °C",0xFFFF); // temp 
  font20(60,170,"0 %",0xFFFF); // heart rate
  font20(60,210,"0 bpm",0xFFFF); // spo2
  stepDetector.begin();
  tft.sleep(false);
  delay(20000);
  Serial.print("Stack high-water mark: ");
  Serial.println(getStackHighWaterMark());
}

void loop() {
  SPO2_HR_handle();
  stepDetector.update();
  stepDetector.Sleeping();
  unsigned long elapsedTime = millis() - lastMeasurementTime;
  unsigned long elapsedDisplay = millis() - lastDisplayTime;
  int walking = stepDetector.getStepCount();
  float targetTemp = particleSensor.readTemperature();
  int totalsleep = stepDetector.getSleep();
  int lightsleep = stepDetector.getlightSleep();
  int deepsleep = stepDetector.getdeepSleep();
  int totallight = stepDetector.gettotallightSleep();
  int totaldeep = stepDetector.gettotaldeepSleep();
  long collect = stepDetector.getCollect();
  int light = stepDetector.getLight();
  int batteryVoltage = Battery();
  dis_state = stepDetector.getAwake();
  if (dis_state == true){
    tft.sleep(false);
  }
  if (validSPO2  == 1 and validHeartRate == 1) {
    totalSpO2 = totalSpO2 + spo2;
    totalHr = totalHr + heartRate;
    validSpO2Count = validSpO2Count + 1 ;
  }
  else{
    totalSpO2 = totalSpO2  + 1;
    totalHr =  totalHr + 1; 
    validSpO2Count = validSpO2Count + 1 ;
  }
  if (elapsedDisplay >= dis_Interval){
    tft.sleep(true);
    lastDisplayTime = millis();
    dis_state = false;
  }
  if ((elapsedTime >= measurementInterval)) {
    float averageSpO2 = (totalSpO2 / validSpO2Count) <= 100 ? (totalSpO2 / validSpO2Count) : 100;
    float averageHR = totalHr /(validSpO2Count);

    String walktext = String(walking) + " Steps";
    String temptext = String(targetTemp, 2) + " °C";  
    String spo2text = String(averageSpO2, 1) + " %";   
    String hrtext = String(averageHR,1) + " bpm"; 
    String lighttxt = "Light Sleep " + String(lightsleep);
    String deeptxt = "Deep Sleep " + String(deepsleep);
    String b_text = String(batteryVoltage) + "%";

    StaticJsonDocument<256> json;
    json["Temp"] = String(targetTemp);
    json["Walking"] = String(walking);
    json["SpO2"] = String(averageSpO2);
    json["Heartrate"] = String(averageHR);
    json["DeepSleep"] = String(totaldeep);
    json["LightSleep"] = String(totallight);
    json["TotalSleep"] = String(totaldeep);
    // Serialize JSON to a string
    serializeJson(json, jsonString);
    httpClient.post("/data", "application/json", jsonString);

    tft.pushImage(0,20,240,280,bg);
    font16(130,70,b_text,0xFFFF);
    font20(60,90,walktext,0xFFFF); //step
    font20(60,130,temptext,0xFFFF); // temp 
    font20(60,170,hrtext,0xFFFF); // heart rate
    font20(60,210,spo2text,0xFFFF); // spo2

    json.clear();
    jsonString.clear();
    lastMeasurementTime = millis();
    totalSpO2 = 0;
    totalHr = 0;
    validSpO2Count = 0;
  }
}