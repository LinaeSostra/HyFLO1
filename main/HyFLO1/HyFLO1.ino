/*
 * Written By: Neutron Her and Rebecca Dun
 * 
 * The purpose of this script is to automate the HyFLO 1 to center over an arbitrary cup, and 
 * dispense the user's preference of liquid using Fuzzy Logic.
 */
#include "libraries/SparkFun_VL6180X.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Time of Flight Sensor
#define TIME_OF_FLIGHT_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 210 // mm

VL6180x sensor(TIME_OF_FLIGHT_ADDRESS);

// Ultrasonic Sensor
/*
 * The ultrasonic sensor requires 2 pins (http://wiki.jmoon.co/hcsr04/):
 * Trigger Pin is used to send out an ultrasonic high level pulse for at least 10 microseconds
 * Echo Pin automatically detects the returning pulse, measured in microseconds
 */
#define triggerPin 10
#define echoPin 9

#define SPEED_OF_SOUND 0.343 // mm per microsecond
#define DETECTION_THRESHOLD 100 // mm
#define MAX_ERROR_PERCENTAGE 0.1 // % (unitless)

#define TRIGGER_SWITCH_WAITTIME 2 // microseconds
#define TRIGGER_PULSE_WAITTIME 10 // microseconds
#define CONTAINER_DEBOUNCE_WAITTIME 1000 // milliseconds

// Stepper Motor Easy Driver
#define stepperPin 4 // Rising edge (LOW -> HIGH) triggers a step
#define directionPin 5 // Set LOW to step 'forward', Set HIGH to step 'backwards'
#define enablePin  6 // Controls whether GND is enabled

#define STEPPER_SWITCH_WAITTIME 70 // microseconds

int stepCounter = 0; 

// Tactile Position Switches
// These tactile switches are HIGH when not pressed, and LOW when pressed
#define homePin 7
#define endPin 8

bool hasReturnedHome = false;

// Pump
#define pumpPin 11

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

const int RIM_THRESHOLD_STEPS = 15;
const int MINIMUM_CUP_HEIGHT = 10; // mm
const double HEIGHT_DROP_PERCENTAGE = 0.3; // % (unitless)

int rimLocation, rimLocation2 = 0;
int rimHeight, rimHeight2 = 0;
bool isFirstRimLocated, isSecondRimLocated = false;
bool hasPassedFirstRim = false;

bool isNozzleCentered = false;
bool isScanComplete = false;
bool hasFinishedDispensing = false;

void setup() {
  Serial.begin(BAUD_RATE);

  // Initialize Time of Flight Sensor
  if(sensor.VL6180xInit() != 0) {
    Serial.println("FAILED TO INITIALIZE");
  }
  sensor.VL6180xDefautSettings();

  delay(1000);

  // Initialize Ultrasonic Sensor
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize Stepper Motor / Easy Driver
  pinMode(stepperPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  resetDriver();
 
  // Initalize Tactile Position Switches
  pinMode(homePin, INPUT_PULLUP);
  pinMode(endPin, INPUT_PULLUP);

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
    StepForward();
    findAllRims();
    // Check if scan is complete
    bool isAtEndPosition = digitalRead(endPin) == LOW;
    bool areAllRimsLocated = isFirstRimLocated && isSecondRimLocated;
    if(isAtEndPosition || areAllRimsLocated) {
      #ifdef DEBUG
      Serial.print("Rim 1 (Height, Location): "); Serial.print(rimHeight); Serial.print(", "); Serial.println(rimLocation);
      Serial.print("Rim 2 (Height, Location): "); Serial.print(rimHeight2); Serial.print(", "); Serial.println(rimLocation2);
      #endif
      isScanComplete = true;
      //Serial.print("Total Steps: "); Serial.println(stepCounter);
      resetDriver();
      break;
    }
  }

  bool isReadyToCenterNozzle = isContainerThere && isScanComplete && !isNozzleCentered;
  while (isReadyToCenterNozzle) {
    StepReverse();
    int containerLocation = calculateCenterOfContainer();
    //TODO(Rebecca): Add error range of feasible stepCounter to containerLocation
    bool hasReachedCenterLocation = stepCounter == containerLocation;
    if (hasReachedCenterLocation) {
      isNozzleCentered = true;
      resetDriver();
      break;
    }
  }
  
  bool isReadyToDispenseLiquid = isContainerThere && isScanComplete && isNozzleCentered && !hasFinishedDispensing;
  while(isReadyToDispenseLiquid) {
    //analogWrite(pumpPin, 255);
    delay(1000); // BE SUPER CAREFUL WITH THIS!!!
    // TODO(Rebecca): Add Half Fill Functionality
    analogWrite(pumpPin, 0);
    delay(2000);
    hasFinishedDispensing = true;
    hasReturnedHome = false;
    break;
  }

  if(!isContainerThere) {
    resetSystem();
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////

/* Reset Easy Driver pins to default state by:
  - resetting the stepper (LOW)
  - the direction to move "forward" (LOW)
  - Enabling GND (HIGH)
*/
void resetDriver() {
  digitalWrite(stepperPin, LOW);
  digitalWrite(directionPin, LOW);
  digitalWrite(enablePin, HIGH);
}

// Sets the Easy Driver pins to 'forward' direction
void setDriverForward() {
  digitalWrite(directionPin, LOW);
  digitalWrite(enablePin, LOW);
}

// Set the Easy Driver pins to 'reverse' direction
void setDriverReverse() {
  digitalWrite(directionPin, HIGH);
  digitalWrite(enablePin, LOW);
}

// Step the motor once by pulling the stepperPin high, then low
void stepOnce() {
  digitalWrite(stepperPin, HIGH);
  delayMicroseconds(STEPPER_SWITCH_WAITTIME); //2000 was best
  digitalWrite(stepperPin, LOW);
  delayMicroseconds(STEPPER_SWITCH_WAITTIME); //2000 was best
}

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

void StepForward() {
  setDriverForward();
  for (int i = 0; i < 100 ; i++) {
    stepOnce();
  }
  stepCounter++;
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
#endif
}

bool findRim(bool isFirstRim) {
  bool isRimLocated = isFirstRim ? isFirstRimLocated : isSecondRimLocated;
  int height = isFirstRim ? rimHeight : rimHeight2;
  int location = isFirstRim ? rimLocation : rimLocation2;
  
  if(!isRimLocated){
    int hacking = 30;
    bool hasPassedSketchyRegion = stepCounter > hacking; // This sketchy region won't be an issue with the new rig.
    bool isReasonableHeight = averageHeight > MINIMUM_CUP_HEIGHT;

    double rimError = abs(rimHeight - averageHeight)/double(rimHeight);
    #ifdef DEBUG
      Serial.print("Rim Error: "); Serial.println(rimError);
    #endif
    if (!isFirstRim && rimError > HEIGHT_DROP_PERCENTAGE) {
        hasPassedFirstRim = true;
      }

    if(hasPassedSketchyRegion && isReasonableHeight) {
      
      bool hasFoundNewRim = averageHeight > height;
      if(hasFoundNewRim) {
        if (isFirstRim) {
          updateRimParameters(isFirstRim, averageHeight, stepCounter);
        } else {
          #ifdef DEBUG
            Serial.print("Rim Difference [Rim 1, Rim2]: "); Serial.print(rimHeight); Serial.print(", "); Serial.println(averageHeight);
            Serial.print("Rim Error: "); Serial.println(rimError);
            Serial.print("Passed First Rim?: "); Serial.println(hasPassedFirstRim);
          #endif

          if (rimError < MAX_ERROR_PERCENTAGE && hasPassedFirstRim) {
            updateRimParameters(isFirstRim, averageHeight, stepCounter);
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
  int rimStabilizedCounter = (location == 0) ? 0 : (stepCounter - location);
  bool hasRimStabilized = rimStabilizedCounter >= RIM_THRESHOLD_STEPS;
  return hasRimStabilized ? true : false;
}

void updateRimParameters(bool isFirstRim, int height, int location) {
#ifdef DEBUG
  Serial.print("Is this the first Rim? "); Serial.println(isFirstRim);
  Serial.print("newRimHeight = "); Serial.println(height);
  Serial.print("newRimLocation = "); Serial.println(location);
#endif

  if (isFirstRim) {
    rimHeight = height;
    rimLocation = location;
  } else {
    rimHeight2 = height;
    rimLocation2 = location;
  }
}
 
// Reverse default microstep mode function
void StepReverse() {
  setDriverReverse();
  for (int i = 0; i < 100; i++) {
    stepOnce();
  }
  stepCounter--;
}

// Sends the nozzle to the home position
void returnHome() {
  while(!hasReturnedHome) {
    StepReverse();
    bool isAtHomePosition = digitalRead(homePin) == LOW;
    if (isAtHomePosition) {
      hasReturnedHome = true;
      stepCounter = 0; 
    }
  }
}

int calculateCenterOfContainer() {
  if (rimLocation == 0 || rimLocation2 == 0) {
    resetSystem();
    return 0;
  }
  // Note: This is likely overkill, but to prevent integer overflow
  return (rimLocation / 2 + rimLocation2 / 2);
}

// Reset the system to idle ready
void resetSystem() {
  analogWrite(pumpPin, 0);
  hasFinishedDispensing = false;
  isScanComplete = false;
  isNozzleCentered = false;
  hasReturnedHome = false;
  resetDriver();
}

// Returns the time of flight reading as a height
int getTimeOfFlightReading() {
  int height = TIME_OF_FLIGHT_MAX_DISTANCE - sensor.getDistance();
  //assert(height >= 0);
  return height;
}

// Returns the ultrasonic distance reading as a distance
int getUltrasonicReading() {
  digitalWrite(triggerPin, LOW); 
  delayMicroseconds(TRIGGER_SWITCH_WAITTIME); // Waiting to update to LOW
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(TRIGGER_PULSE_WAITTIME); // Waiting for return signal
  digitalWrite(triggerPin, LOW);

  // Converting ultrasonic reading to distance
  //
  // Note: To convert the ultrasonic measurement from time (microseconds) to distance (mm),
  // the time for the wave to return needs to be halved as it presents both the time to hit
  // the object and return back to the sensor. Then this time needs to multipled by 
  // the speed of sound. All of this is calculated as a float, and then casted into an int
  // to prevent integer overflow/negative distance readings.
  //
  // distance = ((Time for wave to return / 2) * Speed of Sound)
  int distance = (int) (pulseIn(echoPin, HIGH) / 2 * SPEED_OF_SOUND);
  return distance;
}

// Checks whether an object has been placed in the vicinity or not
bool checkForContainer() {
  bool isContainerThere = false;
  int ultrasonicDistance = getUltrasonicReading();
  int temp = getUltrasonicReading();
#ifdef DEBUG
  Serial.print("Ultrasonic Distance: "); Serial.println(ultrasonicDistance);
  Serial.print("Temp Distance: "); Serial.println(temp);
#endif
  
  bool isObjectPresent = ultrasonicDistance < DETECTION_THRESHOLD;
  if (isObjectPresent) {
    // Debounce distance checking
    delay(CONTAINER_DEBOUNCE_WAITTIME);
    int ultrasonicDistance2 = getUltrasonicReading();
    bool isDistancePositive = ultrasonicDistance > 2;
    double errorPercentage = isDistancePositive ? (abs(ultrasonicDistance - ultrasonicDistance2) / double(ultrasonicDistance)) : 0; 
    
    // Checking if distance has stabilized
    bool hasDistanceStabilized = errorPercentage < MAX_ERROR_PERCENTAGE;
    if (hasDistanceStabilized) {
      isContainerThere = true;
    }
  }
  return isContainerThere;
}
