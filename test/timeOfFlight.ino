/*
 * Time of Flight
 */

#include "Pins.h"

////////////////////////////////
/* Functions */
////////////////////////////////
uint16_t getDistance() {
  VL53L0X_RangingMeasurementData_t measure;

  timeOfFlight.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  bool checkingIncorrectData = (measure.RangeStatus != 4); // phase failures have incorrect data
  return checkingIncorrectData ? measure.RangeMilliMeter : 0;
}

void testTimeOfFlight() {
  uint16_t distance = getDistance();
  if(distance == 0) {
    Serial.println("Out of Range.");
  } else { 
    Serial.print("Distance (mm): "); Serial.println(distance);
  }
}
