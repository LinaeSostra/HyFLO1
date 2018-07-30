/*
 * The purpose of this script is to test each sensor/actuator 
 * works as intended.
 * 
 * Written By: Rebecca Dun
 * -------------------------------------------------------------
 *
 * -------------------------------------------------------------
 */
//#include "src/VL53L0X/Adafruit_VL53L0X.h"
#include "src/VL6180X/SparkFun_VL6180X.h"
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
  //testSensorsAndActuators(); // To test each sensor and actuator
  startSystem(); // To start dispensing sequence
}

void testSensorsAndActuators() {
  // This is working as intended if the ultrasonic values fluctate between ~ 30 - 5000 mm
  // Anything out of this range is likely hitting out of range values
  //Serial.println("\nUltrasonic Testing");
  //testUltrasonic();

  // This is working as intended if the time of flight values fluctate between ~ 30 - 250 mm
  // Anything out of this range is likely hitting out of range values
  //Serial.println("\nTime of Flight Testing");
  //testTimeOfFlight();
  //plotRawTimeOfFlight(); // Prints the raw time of flight readings in serial plotter
  //plotTimeOfFlight(); // Prints the values to be read in serial plotter
  //plotAverageTimeOfFlight(); // Prints the average values to read in serial plotter

  //Serial.println("\nMotor Testing");
  //testMotorAndSwitches(); // Tests the motor moves to the switches and stops
  //testMotor(); // Tests the motor moves forwards and backwards 
  

  //Serial.println("\nPump Testing");
  //testPump(); // WARNING!! Turns on the pump at max dispensing for 5 seconds.
  //testPump2();
  //testPumpRealTime(2500);

  delay(100);
}

// Runs the system through a scan profile of a cup when a cup is placed.
void startSystem() {
  bool isContainerThere = checkForContainer();
  bool hasMotorStarted = getMotorFlag();
  bool shouldScan = isContainerThere && !hasMotorStarted;
  bool shouldReset = !isContainerThere && hasMotorStarted;
  
  if (shouldScan) {
    testMotorAndSwitches();
  }

  // Reset System
  if(shouldReset) {
    #ifdef DEBUG
      Serial.println("SYSTEM RESETTING!");
    #endif
    resetSwitches();
    resetMotorFlag();
  }
}
