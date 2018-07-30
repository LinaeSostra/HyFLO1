/*
 * Written By: Neutron Her and Rebecca Dun
 * 
 * The purpose of this script is to automate the HyFLO 1 to center over an arbitrary cup, and 
 * dispense the user's preference of liquid using Fuzzy Logic.
 */
#include "src/VL53L0X/Adafruit_VL53L0X.h"
#include "src/VL6180X/SparkFun_VL6180X.h"
#include "src/NewPing/NewPing.h"
#include "Pins.h"

//#define DEBUG // comment this line to disable debug (Serial Prints)

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600

bool isScanComplete = false;
bool isNozzleCentered = false;
bool hasFinishedDispensing = false;


void setup() {
  Serial.begin(BAUD_RATE);

  while(!Serial) { delay(1); }

  switchSetup(); // Initalize Tactile Position Switches
  ultrasonicSetup(); // Initialize Ultrasonic Sensor
  timeOfFlightSetup(); // Initialize Time of Flight Sensor

  motorSetup(); // Initialize Stepper Motor / Easy Driver
  pumpSetup();  // Initialize Pump
}

// MAIN LOOP /**********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void loop() {
  // Send the nozzle to home position 
  returnHome();  
  
  // Check if there's a container present
  bool isContainerThere = checkForContainer();
  
  // Once container is present, start scan
  bool isReadyToScan = isContainerThere && !isScanComplete;
  while (isReadyToScan) {
    stepForward();
    findAllRims();
    // Check if scan is complete
    if(hasVisitedEnd() || areAllRimsLocated()) {
      #ifdef DEBUG
      Serial.print("Rim 1 (Height, Location): "); Serial.print(getRimHeight()); Serial.print(", "); Serial.println(getRimLocation());
      Serial.print("Rim 2 (Height, Location): "); Serial.print(getRim2Height()); Serial.print(", "); Serial.println(getRim2Location());
      Serial.print("Total Steps: "); Serial.println(getStepCount());
      #endif
      isScanComplete = true;
      resetEasyDriver();
      resetSwitches();
      break;
    }
  }

  bool isReadyToCenterNozzle = isContainerThere && isScanComplete && !isNozzleCentered;
  while(isReadyToCenterNozzle) {
    int containerLocation = calculateCenterOfContainer();
    goToLocation(containerLocation);
    isNozzleCentered = (containerLocation != 0)? true: false;
    #ifdef DEBUG
    Serial.print("Step Count: ");Serial.println(getStepCount());
    Serial.print("Is Nozzle Centered? "); Serial.println(isNozzleCentered);
    #endif
    break;
  }
  
  bool isReadyToDispenseLiquid = isContainerThere && isScanComplete && isNozzleCentered && !hasFinishedDispensing;
  while(isReadyToDispenseLiquid) {
    //testPumpRealTime(2500);
    startDispensing();
    hasFinishedDispensing = true;
    resetSwitches();
    break;
  }

  if(!isContainerThere && isScanComplete) {
    #ifdef DEBUG
      Serial.println("RESETTING SYSTEM!!!");
    #endif
    resetSystem();
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////

// Reset the system to idle ready
void resetSystem() {
  hasFinishedDispensing = false;
  isNozzleCentered = false;
  isScanComplete = false;
  
  resetRimDetection();
  resetEasyDriver();
  resetDispensing();
}
