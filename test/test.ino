/*
 * The purpose of this script is to test each sensor/actuator 
 * works as intended.
 * 
 * Written By: Rebecca Dun
 * -------------------------------------------------------------
 *
 * -------------------------------------------------------------
 */

#include "src/VL53L0X/Adafruit_VL53L0X.h"
#include "src/NewPing/NewPing.h"
#include "Pins.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

void setup() {
  Serial.begin(BAUD_RATE);

  while(!Serial) {
    delay(1);
  }

  // Sensor Setup
  switchSetup();
  ultrasonicSetup();
  timeOfFlightSetup();

  // Actuator Setup
  motorSetup();
  pumpSetup();
}

void loop() {
  testSensorsAndActuators(); // To test each sensor and actuator
  //startSystem(); // To start dispensing sequence
}

void testSensorsAndActuators() {
  // This is working as intended if the ultrasonic values fluctate between ~ 30 - 5000 mm
  // Anything out of this range is likely hitting out of range values
  Serial.println("\nUltrasonic Testing");
  testUltrasonic();

  // This is working as intended if the time of flight values fluctate between ~ 30 - 1300 mm
  // Anything out of this range is likely hitting out of range values
  Serial.println("\nTime of Flight Testing");
  testTimeOfFlight();

  Serial.println("\nMotor Testing");
  testMotor();

  //Serial.println("\nPump Testing");
  //testPump();

  delay(100);
}

void startSystem() {
  //TODO
}
