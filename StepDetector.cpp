#include "StepDetector.h"

StepDetector::StepDetector() {
  stepThreshold = 100;
  displayThreshold = 20;
  stepCount = 0;
  vectorPrevious = 0;
  vector = 0;
  //dis_awake = false;
}

void StepDetector::begin() {
  Wire.begin();
  mySensor.setWire(&Wire);
  uint8_t sensorId;
  while (mySensor.readId(&sensorId) != 0) {
    Serial.println("Cannot find device to read sensorId");
    delay(2000);
  }
  mySensor.beginGyro();
}

void StepDetector::update() {
  mySensor.gyroUpdate();
  float mX = mySensor.gyroX();
  float mY = mySensor.gyroY();
  float mZ = mySensor.gyroZ();

  // Step detection algorithm
  vector = sqrt((mX * mX) + (mY * mY) + (mZ * mZ));
  float totalVector = vector - vectorPrevious;

  if (vector > stepThreshold) {
    stepCount++;
  }

  if (vector > displayThreshold) {
    dis_awake = true;
  } else {
    dis_awake =false;
  }
  vectorPrevious = vector;
  //vector = 0;
}

void StepDetector::Sleeping(){
  mySensor.gyroUpdate();
  if (millis() - timer > 1000) {
    x = mySensor.gyroX();
    y = mySensor.gyroY();
    z = mySensor.gyroZ();

    if (activate == 0) { // first sleep confirmation
      if ((x <= 20 || x >= -20) && (y <= 20 || y >= -20) && (z <= 20 || z >= -20)) {
        sleep_timer_start = millis() / 1000 - sleep_timer_end;
        if (sleep_timer_start >= 300) { //300
          activate = 1;
        }
      }
      if ((x >= 20 || x <= -20) || (y >= 20 || y <= -20) || (z >= 20 || z <= -20)) {
        sleep_timer_end = (millis() / 1000);
      }
    }

    if (activate == 1) { // sleeping mode
      light_sleep = (millis() / 1000) - sleep_timer_end;

      if (interrupt == 0) {
        if (light_sleep >= 4200) {
          if (interrupt_for_deep_sleep > 4200) {
            if (light_sleep - interrupt_sleep_timer >= 600) {
              deep_sleep = light_sleep - interrupt_for_deep_sleep;
            }
          }
        }
      }
      light_sleep = light_sleep - deep_sleep;

      if ((x >= 20 || x <= -20) || (y >= 20 || y <= -20) || (z >= 20 || z <= -20)) {
        interrupt_sleep_timer = (millis() / 1000) - sleep_timer_end;
        interrupt_for_deep_sleep = light_sleep;
        interrupt = interrupt + 1;
        delay(8000);
      }

      if ((millis() / 1000) - sleep_timer_end - interrupt_sleep_timer > 300) {
        interrupt = 0;
      }

      if ((millis() / 1000) - sleep_timer_end - interrupt_sleep_timer <= 300) {
        if (interrupt >= 5) {
          sleep_timer_end = (millis() / 1000);
          if (light_sleep >= 900) { // second sleep confirmation
            total_light_sleep = total_light_sleep + light_sleep;
            total_deep_sleep = total_deep_sleep + deep_sleep;
            total_sleep = total_light_sleep + total_deep_sleep;
          }
          activate = 0;
          interrupt = 0;
          deep_sleep = 0;
          light_sleep = 0;
          interrupt_sleep_timer = 0;
          interrupt_for_deep_sleep = 0;
        }
      }
    }
    stage_sleep_time = light_sleep + deep_sleep;
    if (stage_sleep_time >= 5400) {
      sleep_timer_end = (millis() / 1000);
      total_light_sleep = total_light_sleep + light_sleep;
      total_deep_sleep = total_deep_sleep + deep_sleep;
      total_sleep = total_light_sleep + total_deep_sleep;
      activate = 0;
      interrupt = 0;
      deep_sleep = 0;
      light_sleep = 0;
      interrupt_sleep_timer = 0;
      interrupt_for_deep_sleep = 0;
    }

    timer = millis();
  }
}

int StepDetector::getStepCount() {
  return stepCount;
}

int StepDetector::getlightSleep() {
  if (light_sleep >= 900) {
    return light_sleep / 60;
  }
  return 0;
}

int StepDetector::getdeepSleep() {
  if (light_sleep >= 900) {
    return deep_sleep / 60;
  }
  return 0;
}

int StepDetector::gettotallightSleep() {
  return total_light_sleep / 60;
}

int StepDetector::gettotaldeepSleep() {
  return total_deep_sleep / 60;
}

int StepDetector::getSleep() {
  return total_sleep / 60;
}

long StepDetector::getCollect() {
  return sleep_timer_start;
}

bool StepDetector::getAwake() {
  return dis_awake;
}

float StepDetector::getVector() {
  return vector;
}

int StepDetector::getLight() {
  return light_sleep;
}