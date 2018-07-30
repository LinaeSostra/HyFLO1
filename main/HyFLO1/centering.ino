/* Centering Algorithm for Containers */

// Globals
const int RIM_THRESHOLD_STEPS = 10;
const int MINIMUM_CUP_HEIGHT = 10; // mm
const int RIM_DIFFERENCE = 15; // mm
const int NOZZLE_OFFSET_STEP = 4; // 100 steps = 4 mm -> 4*4mm ~ 1.6 cm
const int MAX_CENTERING_ATTEMPTS = 2;

// Variables
int rimLocation, rimLocation2 = 0;
int rimHeight, rimHeight2 = 0;
bool isFirstRimLocated, isSecondRimLocated = false;
bool hasPassedFirstRim = false;
int numOfCenteringAttempts = 0;

// Functions

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
  int averageHeight = getAverageHeight();
  
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

bool areAllRimsLocated() {
  return isFirstRimLocated && isSecondRimLocated;
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
  // If failed to find both rims of the container, return 0
  if (rimLocation == 0 || rimLocation2 == 0) {
    return 0;
  }
  return ((rimLocation + rimLocation2) / 2) - NOZZLE_OFFSET_STEP;
}

int calculateAverageContainerHeight() {
  if(rimHeight == 0 || rimHeight2 == 0) {
    return 0;
  }
  return ((rimHeight + rimHeight2) / 2);
}

void updateScanAttempts() {
  numOfCenteringAttempts++;
  #ifdef DEBUG
    Serial.print("Updated numOfAttempts: ");Serial.println(numOfCenteringAttempts);
  #endif
}

bool canScanAgain() {
  return (numOfCenteringAttempts < MAX_CENTERING_ATTEMPTS);
}

void resetScanAttempts() {
  numOfCenteringAttempts = 0;
}

void resetRimDetection() {
  isFirstRimLocated = false;
  isSecondRimLocated = false;
  hasPassedFirstRim = false;
  
  rimLocation = 0;
  rimLocation2 = 0;
  rimHeight = 0;
  rimHeight2 = 0;
}

// Getter Functions
int getRimHeight() {
  return rimHeight;
}

int getRim2Height() {
  return rimHeight2;
}

int getRimLocation() {
  return rimLocation;
}

int getRim2Location() {
  return rimLocation2;
}
