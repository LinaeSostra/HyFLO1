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

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

bool isNozzleCentered = false;
bool isScanComplete = false;
bool hasFinishedDispensing = false;
//TODO(Rebecca): This will be from the terminal!!


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
    Serial.println("Pretend Dispensing");
    int fillSelection = getFillSelection();
    switch(fillSelection) {
      case 0:
        Serial.println("No Fill has been Selected");
        analogWrite(pumpPin, 0);
        break;
      case 1:
        Serial.println("Filling to half Cup");
        break;
      case 2:
        Serial.println("Filling 3/4ths of Cup");
        //analogWrite(pumpPin, 191);
        break;
      case 3:
        Serial.println("Filling to full Cup");
        //analogWrite(pumpPin, 255);
        break;
    }
    delay(1000); // BE SUPER CAREFUL WITH THIS!!!
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
