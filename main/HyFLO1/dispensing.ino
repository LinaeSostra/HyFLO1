/* Logic for Dispensing */
// Rudimentary Fuzzy Controller

#include "Enums.h"

//TODO(Rebecca): Tune
const int shortHeightThreshold = 50;
const int mediumHeightThreshold = 75;
const int tallHeightThreshold = 95;

struct fillTime {
  long noFullCupTime;
  long halfCupTime;
  long threeQuartersCupTime;
  long fullCupTime;
  FillSpeed fillSpeed;
};

const struct fillTime errorCupTime = { 0, 0, 0, 0, zero };
const struct fillTime smallCupTime = { 0, 2500, 3000, 3500, half };
const struct fillTime mediumCupTime = { 0, 4000, 5500, 7500, full };
const struct fillTime largeCupTime = { 0, 6500, 8500, 1150, full };
const struct fillTime extraLargeCupTime = { 0, 7500, 12000, 15500, full };

const struct fillTime cupTimes[5] = { errorCupTime, smallCupTime, mediumCupTime, largeCupTime, extraLargeCupTime };

// Local Variables
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

long getDispenseTime(struct fillTime cupTime) {
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
  FillSpeed dispenseSpeed = cupTime.fillSpeed;
  const long dispenseTime = getDispenseTime(cupTime);

  #ifdef DEBUG
    Serial.print("Dispense Speed: "); Serial.println(dispenseSpeed);
    Serial.print("Dispense Time: "); Serial.println(dispenseTime);
  #endif

  // DO NOT UNCOMMENT UNLESS CONFIDENT IT GONNA WORK!!
  dispensePump(dispenseSpeed, dispenseTime);
}

void resetDispensing() {
  cupHeight = noHeight;
  fillAmount = noFill;
  pumpOff();
  resetPumpFlag();
}
