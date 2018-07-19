 /*
 * Stepper Motor Easy Driver 
 */

// Global Constants
const int STEPPER_SWITCH_WAITTIME = 100; // microseconds

int stepCounter = 0; 
bool wasMotorOn = false;

// Initialize Stepper Motor / Easy Driver Pins & set driver to default
void motorSetup() {
  pinMode(stepperPin, OUTPUT);
  pinMode(directionPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  
  resetEasyDriver();
  Serial.println("Motor Control Setup Complete!");
}

void resetStepperCount() {
  stepCounter = 0;
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
/*
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
}*/

int x;
int y;
int state;

// Testing the stepper motor by going forward 10 times, then backwards 10 times
void testMotor() {
  if(!wasMotorOn) {
  Serial.println("Alternate between stepping forward and reverse.");
  digitalWrite(enablePin, LOW);
  for(x= 1; x<5; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    //Read direction pin state and change it
    state=digitalRead(directionPin);
    if(state == HIGH)
    {
      digitalWrite(directionPin, LOW);
    }
    else if(state ==LOW)
    {
      digitalWrite(directionPin, HIGH);
    }
    
    for(y=1; y<10000; y++)
    {
      digitalWrite(stepperPin, HIGH); //Trigger one step
      delayMicroseconds(STEPPER_SWITCH_WAITTIME);
      digitalWrite(stepperPin, LOW); //Pull step pin low so it can be triggered again
      delayMicroseconds(STEPPER_SWITCH_WAITTIME);
    }
  }
  wasMotorOn = true;
  }else {
    Serial.println("Motor Flag on, so not moving!");
  }
  /*if(!wasMotorOn) { 
    int count = 100;
    //stepReverse();
    Serial.println("Stepping Forward");
    for(int i = 0; i < count; i++) {
      stepForward();
    }
    resetEasyDriver();
    
    Serial.println("Stepping Backward");
    for(int i = 0; i < count; i++) {
      stepReverse();
    }
    
  }
  wasMotorOn = true;*/
}
