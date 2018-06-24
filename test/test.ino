/*
 * The purpose of this script is to test each sensor/actuator 
 * works as intended.
 * 
 * Written By: Rebecca Dun
 */

#include "libraries/VL53L0X/Adafruit_VL53L0X.h"
#include "Pins.h"

Adafruit_VL53L0X timeOfFlight = Adafruit_VL53L0X();

#define DEBUG // comment this line to disable debug (Serial Prints)

void setup() {
  Serial.begin(115200);

  while(!Serial) {
    delay(1);
  }

#ifdef DEBUG
  Serial.println("Adafruit VL53L0X test");
  if(!timeOfFlight.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  Serial.println("Adafruit VL53L0X Booted Up Successfully");
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  testTimeOfFlight();

  delay(100);
}
