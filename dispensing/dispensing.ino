/*
 * Written By: Rebecca Dun
 * 
 * The purpose of this script is to test the HyFLO 1 dispensing
 * of liquid using Fuzzy Logic assuming the nozzle is in centered over 
 * the cup already.
 * 
 * This is tested on an UNO, and not the intended 4udino (Leonardo).
 */

#include "src/VL53L0X/Adafruit_VL53L0X.h"
#include "src/NewPing/NewPing.h"
#include "Pins.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

/* Global Variables */
const int mockRimHeight = 120; //TODO(Rebecca): Change this to dummy value.

bool isNozzleCentered = true;
bool isScanComplete = true;
bool hasFinishedDispensing = false;

void setup() {
  Serial.begin(BAUD_RATE);

  while(!Serial) {
    delay(1);
  }

  // Sensor Setup
  //timeOfFlightSetup();

  // Actuator Setup
  pumpSetup();

  // Fuzzy Logic Setup
  fuzzyLogicSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  //testTimeOfFlight();
  //testPump();

  // Check if there's a container present
  bool isContainerThere = true;

  if(isContainerThere && !getPumpFlag()) {
    dispenseLiquid();
  } else if(!isContainerThere) {
    resetPumpFlag();
  }
}
