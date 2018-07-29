/* Logic for Dispensing */
// Rudimentary Fuzzy Controller

#include "Enums.h"

const int shortHeightThreshold = 30;
const int tallHeightThreshold = 80;

struct fillTime {
  int halfCupTime;
  int threeFourthsCupTime;
  int fullCupTime;
  FillSpeed fillSpeed;
};

const struct fillTime errorCupTime = { 0, 0, 0, zero };
const struct fillTime smallCupTime = { 2500, 3000, 4500, half };
const struct fillTime mediumCupTime = { 4000, 5500, 7500, full };
const struct fillTime largeCupTime = { 6500, 8500, 1175, full };

const struct fillTime cupTimes[4] = { errorCupTime, smallCupTime, mediumCupTime, largeCupTime };

CupHeight cupHeight;

void determineCupHeight() {
  int height = calculateAverageContainerHeight();
  if( height == 0 ) {
    cupHeight = noHeight;
  } else if( height < shortHeightThreshold) {
    cupHeight = small;
  } else if( height > tallHeightThreshold) {
    cupHeight = large;
  } else {
    cupHeight = medium;
  }
}

//TODO(Rebecca): Write this shit.
int getDispenseTime() {
  return -1;
}
