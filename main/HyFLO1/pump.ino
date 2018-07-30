/*
 * Pump
 */

#include "Enums.h"

// Local Variables
bool wasPumpOn = false; // To prevent the pump from being on indefinitely.
unsigned long previousMillis = 0; // to store how long the pump has been on

//Initialize Pump
void pumpSetup() {
  pinMode(pumpPin, OUTPUT);
  pumpOff();
  resetPumpFlag();
}

void resetPumpFlag() { 
  wasPumpOn = false;
}

void pumpOn() {
  if(!wasPumpOn) {
    #ifdef DEBUG
      Serial.println("\nPump is On!\n");
    #endif
    analogWrite(pumpPin, 255);
  } else {
    pumpOff();
  }
}

void pumpHalfOn() {
  if(!wasPumpOn) {
    #ifdef DEBUG
      Serial.println("\nHalf Pump is On!\n");
    #endif
    analogWrite(pumpPin, 127);
  } else {
    pumpOff();
  }
}

void pumpOff() {
  analogWrite(pumpPin, 0);
}

void dispensePump(int fillSpeed, const long dispenseTime) {
  if(wasPumpOn) { return; } // Does Nothing
  #ifdef DEBUG
    Serial.println("Dispensing Starting!!");
  #endif
  switch(fillSpeed) {
    case 0:
      pumpOff();
      delay(int(dispenseTime));
      break;
    case 1:
      pumpHalfOn();
      delay(int(dispenseTime));
      break;
    case 2:
      pumpOn();
      delay(int(dispenseTime));
      break;
  }
  
  // Update time when pump was turned On!
  previousMillis = millis(); // Update time when pump was turned On!
  unsigned long currentMillis, timeDifference;
  bool isContainerThere;
  int scaledTime = int(dispenseTime/4);
  
  while(!wasPumpOn) {
    // Note: To error check while user accidently takes the cup while filling
    // the delay() function cannot be used, so millis() will be used instead
    isContainerThere = checkForContainer();
    currentMillis = millis();
    timeDifference = currentMillis - previousMillis;

    if(timeDifference >= scaledTime || !isContainerThere) {
      #ifdef DEBUG
        Serial.println("End Conditions were met! Stop Dispensing");
      #endif
      pumpOff();
      wasPumpOn = true;
      break;
    }
  }
  delay(500);
}

// Turns the pump on for a second, then off.
void testPump() {
  pumpOn();
  delay(2000);
  pumpOff();
  wasPumpOn = true;
}

// Rebecca testing the PWM functionality works w/ intended pump.
void testPump2() {
  pumpHalfOn();
  delay(2000);
  pumpOff();
  wasPumpOn = true;
}
