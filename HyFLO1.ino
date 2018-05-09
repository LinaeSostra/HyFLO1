/*
 * Written By: Neutron Her and Rebecca Dun
 * 
 * The purpose of this script is to automate the HyFLO 1 to center over an arbitrary cup, and 
 * dispense the user's preference of liquid using Fuzzy Logic.
 */
#include "src/SparkFun_VL6180X.h"

// Time of Flight Sensor
#define VL6180X_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 200

VL6180x sensor(VL6180X_ADDRESS);

// Ultrasonic Sensor
#define trigPin 10
#define echoPin 9

long duration, ultrasonicDistance; 

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
//int x = 0;   // for cup placement delay

bool isScanComplete = false;

int rim1_Location;
bool isFirstRimLocated = false;
int rim1_AfterCounter = 0; 
int rim2_Location;

int rim1_Height = 0;
int rim2_Height = 0;

bool isNozzleCentered = false; 

void setup() {
  //TODO(Rebecca): Ask Neutron about comment vs code.
  Serial.begin(9600); //Start Serial at 115200bps

  // Initialize Time of Flight Sensor
  sensor.VL6180xDefautSettings();
  sensor.VL6180xInit();

  // Initialize Stepper Motor / Easy Driver
  pinMode(stepperPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
 
  // Initalize Tactile Position Switches
  pinMode (homePin, INPUT_PULLUP);
  pinMode (endPin, INPUT_PULLUP);

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
  //Serial.println(ultrasonicDistance);
  
  // cup is placed, so start prelim. scan. 
  while (ultrasonicDistance < 15 && !isScanComplete) {
    //TODO(Rebecca): Testing without x
    /*if (x == 0){
      delay(2000);
      x = 1;
    }*/
    StepForward();
    //Check if scan is complete
    endPosition = digitalRead(endPin);
    if (endPosition == LOW) {
      isScanComplete = true;
      //Serial.print("Total Steps: "); Serial.println(stepCounter);
      homePosition = digitalRead(homePin);
      resetEasyDriver();
      break;
    }
  }
  while (ultrasonicDistance < 10 && isScanComplete && !isNozzleCentered){
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


void checkProximity() {
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  ultrasonicDistance = (duration / 2) / 29.1;

  if (ultrasonicDistance < 15) {
    delay(100);
    digitalWrite(trigPin, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    ultrasonicDistance = (duration / 2) / 29.1;
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
