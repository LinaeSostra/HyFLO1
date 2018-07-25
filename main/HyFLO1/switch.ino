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
  Serial.println("Finished Switch Setup");
}

// If Home Switch Pressed, set home flag true
void homePressed() {
  Serial.println("\nHome Tactile Switch Pressed!");
  hasVisitedHomePosition = true;
  resetStepperCount();
}

// If End Switch Pressed, set end flag true
void endPressed() {
  Serial.println("\nEnd Tactile Switch Pressed!");
  hasVisitedEndPosition = true;
  
  //TODO(Rebecca): This may be incorrect logically.
  //hasVisitedHomePosition = false;
  //returnHome();
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
