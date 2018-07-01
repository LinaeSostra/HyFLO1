/*
 * Pump
 */

// To prevent the pump from being on indefinitely.
bool wasPumpOn = false; 

//Initialize Pump
 void pumpSetup() {
  pinMode(pumpPin, OUTPUT);
 }

 void resetPumpFlag() { 
  wasPumpOn = false;
 }

 void pumpOn() {
  if(!wasPumpOn) {
    #ifdef DEBUG
      Serial.println("\nPump is On!");
    #endif
    analogWrite(pumpPin, 255);
  } else {
    pumpOff();
  }
 }

 void pumpHalfOn() {
  if(!wasPumpOn) {
    #ifdef DEBUG
      Serial.println("\nPump is On!");
    #endif
    analogWrite(pumpPin, 128);
  } else {
    pumpOff();
  }
 }

 void pumpOff() {
  analogWrite(pumpPin, 0);
 }

// Turns the pump on for a second, then off.
 void testPump() {
  pumpOn();
  delay(500);
  pumpOff();
  wasPumpOn = true;
 }

// Rebecca testing the PWM functionality works w/ intended pump.
 void testPump2() {
  pumpHalfOn();
  delay(5000);
  pumpOff();
  wasPumpOn = true;
 }

