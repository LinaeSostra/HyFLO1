/*
 * Pump
 */

// To prevent the pump from being on indefinitely.
bool wasPumpOn = false;
unsigned long previousMillis = 0; // to store how long the pump has been on

//Initialize Pump
 void pumpSetup() {
  pinMode(pumpPin, OUTPUT);
  pumpOff();
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
    analogWrite(pumpPin, 127);
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
  delay(500);
  pumpOff();
  wasPumpOn = true;
 }

void testPumpRealTime(int timeOn) {
  pumpOn();
  previousMillis = millis();
  unsigned long currentMillis, timeDifference;
  while(!wasPumpOn) {
    currentMillis = millis();
    timeDifference = int(currentMillis - previousMillis);

    if(timeDifference >= timeOn) {
      pumpOff();
      wasPumpOn = true;
      break;
    }
  }
}
