/*
 * Pump
 */

// Mockup globals for testing
const float userFill = 0.5;
const float MAX_FILL = 0.9;
const float MIN_FILL = 0.0;

// To prevent the pump from being on indefinitely.
bool wasPumpOn = false; 

//Initialize Pump
 void pumpSetup() {
  pinMode(pumpPin, OUTPUT);
 }

 bool getPumpFlag() {
  return wasPumpOn;
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

 // Calculate how much to fill the cup by height [mm]
int calculateTargetHeightFill(float percentage, int rimAvgHeight) {
  //Edge Case Flooring user request to fill
  if(percentage > MAX_FILL) {
    percentage = MAX_FILL;
  } else if(percentage < MIN_FILL) {
    percentage = MIN_FILL;
  }
  return int(percentage * float(rimAvgHeight));
}

// Returns the error difference to target height
int getHeightDifference(int currentHeight, int targetHeight) {
  return (targetHeight - currentHeight);
}

// Returns the height percentage filled
int getFillPercentage(int currentHeight, int cupHeight) {
  return int(float(currentHeight)/float(cupHeight) * 100);
}

// Dispenses liquid into said cup
void dispenseLiquid() {
  int targetHeight = calculateTargetHeightFill(userFill, mockRimHeight);
  int currentHeight = getTimeOfFlightReading();

  fuzzy->setInput(1, getFillPercentage(currentHeight, mockRimHeight));
  fuzzy->setInput(2, mockRimHeight);
  fuzzy->setInput(3, getHeightDifference(currentHeight, targetHeight));

  fuzzy->fuzzify();

  uint8_t tempSpeed = uint8_t(fuzzy->defuzzify(1));
  if(wasPumpOn == false) {
    analogWrite(pumpPin, tempSpeed);
  }
  if (tempSpeed == 0) {
    wasPumpOn = true;
  }
  
  /*
  // Note: I can't extract this code, or it would be in an inf loop and overflow water everywhere.
  while(currentHeight < desiredHeightFill) {
    pumpOn();
    delay(500);
    currentHeight = getTimeOfFlightReading();
  } 
  pumpOff();
  wasPumpOn = true;
  */
 }
