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

#define DEBUG // comment this line to disable debug (Serial Prints)

#define MAX_ERROR_PERCENTAGE 0.1 // % (unitless)
const int NOZZLE_OFFSET_STEP = 4; // 100 steps = 4 mm -> 4*4mm ~ 1.6 cm

// Other Global Variables

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

bool isNozzleCentered = false;
bool isScanComplete = false;
bool hasFinishedDispensing = false;

void setup() {
  Serial.begin(BAUD_RATE);

  while(!Serial) { delay(1); }

  // Initialize Time of Flight Sensor
  timeOfFlightSetup();

  // Initialize Ultrasonic Sensor
  ultrasonicSetup();

  // Initialize Stepper Motor / Easy Driver
  motorSetup();
 
  // Initalize Tactile Position Switches
  switchSetup();

  // Initialize Pump
  pinMode(pumpPin, OUTPUT);
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
      #endif
      isScanComplete = true;
      Serial.print("Total Steps: "); Serial.println(getStepCount());
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
    //analogWrite(pumpPin, 255);
    Serial.println("Pretend Dispensing");
    delay(1000); // BE SUPER CAREFUL WITH THIS!!!
    // TODO(Rebecca): Add Half Fill Functionality
    analogWrite(pumpPin, 0);
    delay(2000);
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
  analogWrite(pumpPin, 0);
  hasFinishedDispensing = false;
  isNozzleCentered = false;
  isScanComplete = false;
  
  resetRimDetection();
  resetEasyDriver();
}
