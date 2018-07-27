/* Gathers input from the serial com of which fill the user wants */

enum fill {
  noFill,
  halfFill,
  threeQuartersFill,
  fullFill
};

// Variables
enum fill fillSelection = halfFill;//noFill;

// Updates the fill selection based on user request
void updateFillSelection() {
  while(Serial.available()) {
    // Read user input
    char userInput = Serial.read();
    switch(userInput) {
      case '1':
        fillSelection = halfFill;
        break;
      case '2':
        fillSelection = threeQuartersFill;
        break;
      case '3':
        fillSelection = fullFill;
        break;
      default:
        // SOMETHING IS WRONG, SHOULDN'T GO HERE :(
        break;
    }
  }
}

//TODO: This should be based on the selection
bool hasUserSelectedFill() {
  return (fillSelection != noFill);
}

int getFillSelection() {
  return fillSelection;
}

void resetSelection() {
  fillSelection = noFill;
}
