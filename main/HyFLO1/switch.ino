/*
 * Tactile Switches
 */

bool hasVisitedHomePosition = false;
bool hasVisitedEndPosition = false;

// Initalize Tactile Position Switches Pins
void switchSetup() {
  pinMode(homePin, INPUT_PULLUP);
  pinMode(endPin, INPUT_PULLUP);

  // Whenever a switch is pressed, interrupt to update switch flag
  attachInterrupt(digitalPinToInterrupt(homePin), homePressed, RISING);
  attachInterrupt(digitalPinToInterrupt(endPin), endPressed, RISING);
  resetSwitches();
  #ifdef DEBUG
  Serial.println("Finished Switch Setup");
  #endif
}

// If Home Switch Pressed, set home flag true
void homePressed() {
  #ifdef DEBUG
  Serial.println("\nHome Tactile Switch Pressed!");
  #endif
  hasVisitedHomePosition = true;
  hasVisitedEndPosition = false;
  resetStepperCount();
}

// If End Switch Pressed, set end flag true
void endPressed() {
  #ifdef DEBUG
  Serial.println("\nEnd Tactile Switch Pressed!");
  #endif
  hasVisitedEndPosition = true;
}

// Set switch flags to false
 void resetSwitches() {
  hasVisitedHomePosition = false;
  hasVisitedEndPosition = false;
}

bool hasVisitedHome() {
  return hasVisitedHomePosition;
}

bool hasVisitedEnd() {
  return hasVisitedEndPosition;
}
