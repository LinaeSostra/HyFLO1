/* Logic for Dispensing */
// Rudimentary Fuzzy Controller

#include "Enums.h"

//TODO(Rebecca): Tune
const int shortHeightThreshold = 30;
const int mediumHeightThreshold = 50;
const int tallHeightThreshold = 80;

struct fillTime {
  int noFullCupTime;
  int halfCupTime;
  int threeQuartersCupTime;
  int fullCupTime;
  FillSpeed fillSpeed;
};

const struct fillTime errorCupTime = { 0, 0, 0, 0, zero };
const struct fillTime smallCupTime = { 0, 2500, 3000, 3500, half };
const struct fillTime mediumCupTime = { 0, 4000, 5500, 7500, full };
const struct fillTime largeCupTime = { 0, 6500, 8500, 1150, full };
const struct fillTime extraLargeCupTime = { 0, 7500, 12000, 15500, full };

const struct fillTime cupTimes[5] = { errorCupTime, smallCupTime, mediumCupTime, largeCupTime, extraLargeCupTime };

CupHeight cupHeight;
FillAmount fillAmount;

void determineCupHeight() {
  int height = calculateAverageContainerHeight();
  if( height == 0 ) {
    cupHeight = noHeight;
  } else if( height > 0 && height < shortHeightThreshold) {
    cupHeight = small;
  } else if(height >= shortHeightThreshold && height < mediumHeightThreshold) {
    cupHeight = medium;
  } else if (height >= mediumHeightThreshold && height < tallHeightThreshold) {
    cupHeight = large;
  } else {
    cupHeight = extraLarge;
  }
}

void determineFillAmount() {
  updateFillSelection();
  fillAmount = getFillSelection();
}

int getDispenseTime(struct fillTime cupTime) {
  switch(fillAmount) {
    case noFill:
      return cupTime.noFullCupTime;
      break;
    case halfFill:
      return cupTime.halfCupTime;
      break;
    case threeQuartersFill:
      return cupTime.threeQuartersCupTime;
      break;
    case fullFill:
      return cupTime.fullCupTime;
      break;
  }
}

void startDispensing() {
  // Gather cup height and fill amount
  determineCupHeight();
  determineFillAmount();

  // Get the dispense time and speed
  fillTime cupTime = cupTimes[cupHeight];
  int dispenseTime = getDispenseTime(cupTime);
  FillSpeed dispenseSpeed = cupTime.fillSpeed;

  //TODO(Rebecca):
  // Send time and speed to pump.
  // Also need to add if someone removes the cup
}

void resetDispensing() {
  cupHeight = noHeight;
  fillAmount = noFill;
  pumpOff();
}
