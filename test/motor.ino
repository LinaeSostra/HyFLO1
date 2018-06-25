/*
 * Stepper Motor Easy Driver 
 */

// Global Constants
const int STEPPER_SWITCH_WAITTIME = 70; // microseconds

int stepCounter = 0; 

// Initialize Stepper Motor / Easy Driver Pins & set driver to default
void motorSetup() {
  pinMode(stepperPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  resetDriver();
}

void resetStepperCount() {
  stepCounter = 0;
}

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
  delayMicroseconds(STEPPER_SWITCH_WAITTIME);
  digitalWrite(stepperPin, LOW);
  delayMicroseconds(STEPPER_SWITCH_WAITTIME);
}

// Step the motor forward
void stepForward() {
  setDriverForward();
  for (int i = 0; i < 100 ; i++) {
    stepOnce();
  }
  stepCounter++;
}

// Step the motor back
void stepReverse() {
  setDriverReverse();
  for (int i = 0; i < 100; i++) {
    stepOnce();
  }
  stepCounter--;
}

// Sends the nozzle to the home position
void returnHome() {
  while(!hasVisitedHome()) {
    stepReverse();
    if(hasVisitedHome()) {
      break;
    } 
  }
}

// Testing the stepper motor by going forward 10 times, then backwards 10 times
void testMotor() {
  int count = 10;
  
  Serial.println("Stepping Forward");
  for(int i = 0; i < count; i++) {
    stepForward();
  }

  Serial.println("Stepping Backward");
  for(int i = 0; i < count; i++) {
    stepReverse();
  }
}