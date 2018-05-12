/*
 * Written By: Neutron Her and Rebecca Dun
 * 
 * The purpose of this script is to automate the HyFLO 1 to center over an arbitrary cup, and 
 * dispense the user's preference of liquid using Fuzzy Logic.
 */
#include "src/SparkFun_VL6180X.h"

// Time of Flight Sensor
#define VL6180X_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 200 // mm

VL6180x sensor(VL6180X_ADDRESS);

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
#define ERROR_PERCENTAGE 0.05 // % (unitless)

#define TRIGGER_SWITCH_WAITTIME 2 // microseconds
#define TRIGGER_PULSE_WAITTIME 10 // microseconds

// Stepper Motor Easy Driver
#define stepperPin 4 // Rising edge (LOW -> HIGH) triggers a step
#define directionPin 5 // Set LOW to step 'forward', Set HIGH to step 'backwards'
#define enablePin  6 // Controls whether GND is enabled

int stepCounter = 0; 

// Tactile Position Switches
#define homePin 7
#define endPin 8

int8_t endPosition;
int8_t homePosition;

// Other Global Variables
// Rolling Average Smoothing Variables
const int numReadings = 5;     // the number of readings to average
int readings[numReadings];
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

bool isContainerThere = false;
bool isScanComplete = false;

int rim1_Location;
bool isFirstRimLocated = false;
int rim1_AfterCounter = 0; 
int rim2_Location;

int rim1_Height = 0;
int rim2_Height = 0;

bool isNozzleCentered = false; 

void setup() {
  Serial.begin(9600);

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
 
  // Initalize Tactile Position Switches
  pinMode(homePin, INPUT_PULLUP);
  pinMode(endPin, INPUT_PULLUP);

  // Initializing readings array to 0s. (for running average algorithm) 
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  // Check current state of system
  homePosition = digitalRead(homePin);
  endPosition = digitalRead(endPin);

  //TODO(Rebecca): Remove this later.
  digitalWrite(directionPin, HIGH); //Pull direction pin HIGH to move "reverse" (back to home)
}

// MAIN LOOP /**********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void loop() {

  //check if the system is at Home Position. If not, fix it. 
  if (homePosition == HIGH && !isScanComplete) {
    returnHome();
  }

  // check if there's a cup
  checkProximity();
  
  // cup is placed, so start prelim. scan. 
  while (isContainerThere && !isScanComplete) {
    StepForward();
    //Check if scan is complete
    endPosition = digitalRead(endPin);
    if(endPosition == LOW) {
      isScanComplete = true;
      //Serial.print("Total Steps: "); Serial.println(stepCounter);
      homePosition = digitalRead(homePin);
      resetEasyDriver();
      break;
    }
  }
  while (isContainerThere && isScanComplete && !isNozzleCentered){
    StepReverse();
    //Serial.print("Goto Step Counter = "); Serial.println(stepCounter);
    if (stepCounter == (rim1_Location+rim2_Location)/2){
      isNozzleCentered = true;
      resetEasyDriver();
      break;
    }
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////
//TODO(Rebecca): Refactor StepForward & StepBackward to be more modular.
void StepForward() {
  //Serial.println("Moving forward at default step mode.");

  //TODO(Rebecca): This oversteps the first rim by too much. Refactor for real time-ness
  for (int i = 0; i < 100; i++) {
    digitalWrite(stepperPin, HIGH); // Trigger one step forward
    delayMicroseconds(70); // 2000 was best
    digitalWrite(stepperPin, LOW); // Pull step pin low so it can be triggered again
    delayMicroseconds(70); // 2000 was best
  }

  uint8_t ToF_distance = sensor.getDistance();

  total = total - readings[readIndex];
  readings[readIndex] = ToF_distance;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0;
  }

  average = TIME_OF_FLIGHT_MAX_DISTANCE - (total / numReadings); 

  Serial.print("Distance = "); Serial.println(average);
  stepCounter++;

  // Find first maxima
  if(average > rim1_Height && !isFirstRimLocated && stepCounter > 30){
    rim1_Height = average;
    rim1_Location = stepCounter;
    //Serial.print("Rim 1 Location = "); Serial.println(rim1_Location);
  }
  
  if(average < rim1_Height && !isFirstRimLocated){
    rim1_AfterCounter++;
    //Serial.print("R1 AfterCounter = "); Serial.println(rim1_AfterCounter);
    if (rim1_AfterCounter == 120){
      isFirstRimLocated = true;
      rim1_AfterCounter = 0;
      //Serial.println("Rim 1 Location"); Serial.println(rim1_Location);
    }
  }
  
  // Find second Maxima
  if(average > rim2_Height && stepCounter > 10 && stepCounter < 265 && isFirstRimLocated){
    rim2_Height = average;
    rim2_Location = stepCounter;
    //Serial.print("Rim 2 Location"); Serial.println(rim2_Location);
  } 
}
 
//Reverse default microstep mode function
void StepReverse() {
  //Serial.println("Moving reverse at default step mode.");
  digitalWrite(enablePin, LOW); //Pull enable pin low to allow motor control
  digitalWrite(directionPin, HIGH); //Pull direction pin low to move "forward"
  for (int i = 0; i < 100; i++) {
    digitalWrite(stepperPin, HIGH); //Trigger one step forward
    delayMicroseconds(70); //2000 was best
    digitalWrite(stepperPin, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(70); //2000 was best
  }
  stepCounter--;
}

void returnHome() {
  while (homePosition == HIGH) {
    digitalWrite(directionPin, HIGH); // Reverse direction
    digitalWrite(enablePin, LOW); //Pull enable pin low to allow motor control
    StepReverse();
    homePosition = digitalRead(homePin);
    if (homePosition == LOW) {
      digitalWrite(directionPin, LOW); //Pull direction pin low to move "forward"
      digitalWrite(enablePin, LOW); //Pull enable pin low to allow motor control
      stepCounter = 0; 
      break;
    }
  }
}

 // Returns the Ultrasonic Distance Reading
long getUltrasonicReading() {
  digitalWrite(triggerPin, LOW); 
  delayMicroseconds(TRIGGER_SWITCH_WAITTIME); // Waiting to update to LOW
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(TRIGGER_PULSE_WAITTIME); // Waiting for return signal
  digitalWrite(triggerPin, LOW);

  // Converting Ultrasonic Reading to Distance
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


void checkProximity() {
  long ultrasonicDistance = getUltrasonicReading();
  //TODO(Rebecca): Make this an interrupt.
  if (ultrasonicDistance < DETECTION_THRESHOLD) {
    delay(100);
    Serial.println(ultrasonicDistance);
    long ultrasonicDistance2 = getUltrasonicReading();
    //TODO(Rebecca): This error is overkill 
    int error = abs(ultrasonicDistance - ultrasonicDistance2) / ultrasonicDistance;
    if( error < ERROR_PERCENTAGE ) {
      isContainerThere = true;
    }
  }
}

/* Reset Easy Driver pins to default state by:
  - resetting the stepper (LOW)
  - the direction to move "forward" (LOW)
  - Enabling GND (HIGH)
*/
void resetEasyDriver() {
  digitalWrite(stepperPin, LOW);
  digitalWrite(directionPin, LOW);
  digitalWrite(enablePin, HIGH);
}
