/*
 * Pump
 */

// Mockup globals for testing
const float userFill = 1;
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
  for(int8_t i = 0; i < 250; i++) {
    testTimeOfFlight();
    delay(20);
  }
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
int16_t calculateTargetHeightFill(float percentage, int16_t rimAvgHeight) {
  //Edge Case Flooring user request to fill
  if(percentage > MAX_FILL) {
    percentage = MAX_FILL;
  } else if(percentage < MIN_FILL) {
    percentage = MIN_FILL;
  }
  return int(percentage * float(rimAvgHeight));
}

// Returns the height percentage filled
int16_t getFillPercentage(int16_t current) {
  int16_t target = calculateTargetHeightFill(userFill, mockRimHeight);
  
  if (target > mockRimHeight) {
    Serial.println(F("Invalid target to fill cup. This would overfill cup"));
  }
  return int16_t(float(current)/float(target) * 100);
}

// Dispenses liquid into said cup
void dispenseLiquid() {
  int16_t targetHeight = calculateTargetHeightFill(userFill, mockRimHeight);
  int16_t currentHeight = getTimeOfFlightReading();

  int16_t fillPercentage = getFillPercentage(currentHeight);
  
  #ifdef DEBUG
     Serial.println("*****************************************");
    Serial.print("Target Height: "); Serial.println(targetHeight);
    Serial.print("Current Height: "); Serial.println(currentHeight);
    Serial.println("*****************************************");
    Serial.print("Input 1: Fill Percentage: "); Serial.println(fillPercentage);
    Serial.print("Input 2: Height of Cup: "); Serial.println(mockRimHeight);
  #endif

  fuzzy->setInput(1, fillPercentage);
  fuzzy->setInput(2, mockRimHeight);

  fuzzy->fuzzify();

  float tempSpeed = float(fuzzy->defuzzify(1));
  if(wasPumpOn == false) {
    #ifdef DEBUG
      Serial.print("Output: Speed Output: "); Serial.println(tempSpeed);
    #endif 
    //analogWrite(pumpPin, tempSpeed);
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
