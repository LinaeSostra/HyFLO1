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

// Rolling Average Smoothing Variables
const int MAX_SAMPLES = 5;     // the number of readings to average
int readings[MAX_SAMPLES];
int readIndex = 0;              // the index of the current reading
int averageHeight = 0;          // the average

const int RIM_THRESHOLD_STEPS = 10;
const int MINIMUM_CUP_HEIGHT = 15; // mm
const int RIM_DIFFERENCE = 15; // mm

int rimLocation, rimLocation2 = 0;
int rimHeight, rimHeight2 = 0;
bool isFirstRimLocated, isSecondRimLocated = false;
bool hasPassedFirstRim = false;

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

  // Initializing readings array to 0s. (for running average algorithm) 
  for (int thisReading = 0; thisReading < MAX_SAMPLES; thisReading++) {
    readings[thisReading] = 0;
  }
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
    bool areAllRimsLocated = isFirstRimLocated && isSecondRimLocated;
    if(hasVisitedEnd() || areAllRimsLocated) {
      #ifdef DEBUG
      Serial.print("Rim 1 (Height, Location): "); Serial.print(rimHeight); Serial.print(", "); Serial.println(rimLocation);
      Serial.print("Rim 2 (Height, Location): "); Serial.print(rimHeight2); Serial.print(", "); Serial.println(rimLocation2);
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
    resetSystem();
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void smoothReading() {
  int height = getTimeOfFlightReading();

  readings[readIndex] = height;
  int total = getRunningTotal();
  readIndex = (readIndex + 1) % MAX_SAMPLES;
  averageHeight = total / MAX_SAMPLES;

#ifdef DEBUG
  Serial.print("Time of Flight Height = "); Serial.println(averageHeight);
#endif
}

int getRunningTotal() {
  int total = 0;
  for(int i = 0; i < MAX_SAMPLES; i++) {
    total = total + readings[i];
  }
  return total;
}

void findAllRims() {
  smoothReading();

  // Find first rim
  isFirstRimLocated = findRim(true);

  // Find second rim
  if(isFirstRimLocated) {
    isSecondRimLocated = findRim(false);
  }

#ifdef DEBUG
  Serial.print("isFirstRimLocated = "); Serial.println(isFirstRimLocated);
  Serial.print("isSecondRimLocated = "); Serial.println(isSecondRimLocated);
  Serial.print("Steps: "); Serial.println(getStepCount());
#endif
}

bool findRim(bool isFirstRim) {
  bool isRimLocated = isFirstRim ? isFirstRimLocated : isSecondRimLocated;
  int height = isFirstRim ? rimHeight : rimHeight2;
  int location = isFirstRim ? rimLocation : rimLocation2;
  
  if(!isRimLocated){
    int hacking = 30;
    bool hasPassedSketchyRegion = getStepCount() > hacking; // This sketchy region won't be an issue with the new rig.
    bool isReasonableHeight = averageHeight > MINIMUM_CUP_HEIGHT;

    double rimError = abs(rimHeight - averageHeight);
    #ifdef DEBUG
      Serial.print("Rim Height: "); Serial.println(rimHeight);
      Serial.print("Average  Height: "); Serial.println(averageHeight);
      Serial.print("Rim Error: "); Serial.println(rimError);
    #endif
    if (!isFirstRim && rimError > RIM_DIFFERENCE) {
        hasPassedFirstRim = true;
    }

    if(hasPassedSketchyRegion && isReasonableHeight) {
      
      bool hasFoundNewRim = averageHeight > height;
      if(hasFoundNewRim) {
        if (isFirstRim) {
          updateRimParameters(isFirstRim, averageHeight, getStepCount());
        } else {
          #ifdef DEBUG
            Serial.print("Rim Difference [Rim 1, Rim2]: "); Serial.print(rimHeight); Serial.print(", "); Serial.println(averageHeight);
            Serial.print("Rim Error: "); Serial.println(rimError);
            Serial.print("Passed First Rim?: "); Serial.println(hasPassedFirstRim);
          #endif

          if (rimError < RIM_DIFFERENCE && hasPassedFirstRim) {
            updateRimParameters(isFirstRim, averageHeight, getStepCount());
          }
        }
      } else {
        isRimLocated = hasRimStabilized(location);
      }
    }
  }
  return isRimLocated;
}

bool hasRimStabilized(int location) {
  int rimStabilizedCounter = (location == 0) ? 0 : (getStepCount() - location);
  bool hasRimStabilized = rimStabilizedCounter >= RIM_THRESHOLD_STEPS;
  return hasRimStabilized ? true : false;
}

void updateRimParameters(bool isFirstRim, int height, int location) {
#ifdef DEBUG
  Serial.println("******************");
  Serial.print("Is this the first Rim? "); Serial.println(isFirstRim);
  Serial.print("newRimHeight = "); Serial.println(height);
  Serial.print("newRimLocation = "); Serial.println(location);
  Serial.println("******************");
#endif

  if (isFirstRim) {
    rimHeight = height;
    rimLocation = location;
  } else {
    rimHeight2 = height;
    rimLocation2 = location;
  }
}

int calculateCenterOfContainer() {
  if (rimLocation == 0 || rimLocation2 == 0) {
    return 0;
  }
  return ((rimLocation + rimLocation2) / 2) - NOZZLE_OFFSET_STEP;
}

// Reset the system to idle ready
void resetSystem() {
  analogWrite(pumpPin, 0);
  hasFinishedDispensing = false;
  isNozzleCentered = false;
  isScanComplete = false;
  
  isFirstRimLocated = false;
  isSecondRimLocated = false;
  hasPassedFirstRim = false;
  rimLocation = 0;
  rimLocation2 = 0;
  rimHeight = 0;
  rimHeight2 = 0;

  resetEasyDriver();
}
