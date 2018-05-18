/*
 * Written By: Neutron Her and Rebecca Dun
 * 
 * The purpose of this script is to automate the HyFLO 1 to center over an arbitrary cup, and 
 * dispense the user's preference of liquid using Fuzzy Logic.
 */
#include "src/SparkFun_VL6180X.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Time of Flight Sensor
#define TIME_OF_FLIGHT_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 200 // mm

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
#define MAX_ERROR_PERCENTAGE 0.05 // % (unitless)

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

// Other Global Variables

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

// Rolling Average Smoothing Variables
const int AVERAGE_SAMPLE_SIZE = 5;     // the number of readings to average
int readings[AVERAGE_SAMPLE_SIZE];
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

bool isScanComplete = false;

int containerRimLocation, containerRimLocation2;
bool isFirstRimLocated = false;
int rim1_AfterCounter = 0;

int rim1_Height = 0;
int rim2_Height = 0;

bool isNozzleCentered = false; 

void setup() {
  Serial.begin(BAUD_RATE);

  // Initialize Time of Flight Sensor
  sensor.VL6180xDefautSettings();
  sensor.VL6180xInit();

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

  // Initializing readings array to 0s. (for running average algorithm) 
  for (int thisReading = 0; thisReading < AVERAGE_SAMPLE_SIZE; thisReading++) {
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
    // Check if scan is complete
    // TODO(Rebecca): After the 2nd rim is found, the scan is complete. Current code is less optimal as it
    // assumes once it hits the end tactile switch the scan is complete, and that takes more time than needed.
    bool isAtEndPosition = digitalRead(endPin) == LOW;
    if(isAtEndPosition) {
      isScanComplete = true;
      //Serial.print("Total Steps: "); Serial.println(stepCounter);
      resetDriver();
      break;
    }
  }

  bool isReadyToCenterNozzle = isContainerThere && isScanComplete && !isNozzleCentered;
  while (isReadyToCenterNozzle) {
    StepReverse();
    //Serial.print("Goto Step Counter = "); Serial.println(stepCounter);
    int containerLocation = calculateCenterOfContainer();
    //TODO(Rebecca): Add error range of feasible stepCounter to containerLocation
    bool hasReachedCenterLocation = stepCounter == containerLocation;
    if (hasReachedCenterLocation) {
      isNozzleCentered = true;
      resetDriver();
      break;
    }
  }

  //TODO(Rebecca): Add Pump Dispensing Functionality Here.
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

//TODO(Rebecca): Refactor StepForward & StepBackward to be more modular.
void StepForward() {
  setDriverForward();
  //Serial.println("Moving forward at default step mode.");

  //TODO(Rebecca): This oversteps the first rim by too much. Refactor for real time-ness
  for (int i = 0; i < 100; i++) {
    stepOnce();
  }

  smoothReading();

  stepCounter++;

  // Find first maxima
  if(average > rim1_Height && !isFirstRimLocated && stepCounter > 30){
    rim1_Height = average;
    containerRimLocation = stepCounter;
    //Serial.print("Rim 1 Location = "); Serial.println(containerRimLocation);
  }
  
  if(average < rim1_Height && !isFirstRimLocated){
    rim1_AfterCounter++;
    //Serial.print("R1 AfterCounter = "); Serial.println(rim1_AfterCounter);
    if (rim1_AfterCounter == 120){
      isFirstRimLocated = true;
      rim1_AfterCounter = 0;
      //Serial.println("Rim 1 Location"); Serial.println(containerRimLocation);
    }
  }
  
  // Find second Maxima
  if(average > rim2_Height && stepCounter > 10 && stepCounter < 265 && isFirstRimLocated){
    rim2_Height = average;
    containerRimLocation2 = stepCounter;
    //Serial.print("Rim 2 Location"); Serial.println(containerRimLocation2);
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

void smoothReading() {
  uint8_t ToF_distance = sensor.getDistance();

  total = total - readings[readIndex];
  readings[readIndex] = ToF_distance;
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % AVERAGE_SAMPLE_SIZE;
  average = TIME_OF_FLIGHT_MAX_DISTANCE - (total / AVERAGE_SAMPLE_SIZE);

#ifdef DEBUG
  Serial.print("Distance = "); Serial.println(average);
#endif
}

int calculateCenterOfContainer() {
  // Note: This is likely overkill, but to prevent integer overflow
  return (containerRimLocation / 2 + containerRimLocation2 / 2);
}

// Returns the ultrasonic distance reading
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
  
  bool isObjectPresent = ultrasonicDistance < DETECTION_THRESHOLD;
  if (isObjectPresent) {
    // Debounce distance checking
    delay(CONTAINER_DEBOUNCE_WAITTIME);
    int ultrasonicDistance2 = getUltrasonicReading();
    bool isDistancePositive = ultrasonicDistance > 0;
    int errorPercentage = isDistancePositive ? 0 : (abs(ultrasonicDistance - ultrasonicDistance2) / ultrasonicDistance); 
    
    // Checking if distance has stabilized
    bool hasDistanceStabilized = errorPercentage < MAX_ERROR_PERCENTAGE;
    if (hasDistanceStabilized) {
      isContainerThere = true;
    }
  }
  return isContainerThere;
}
