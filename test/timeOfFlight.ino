/*
 * Time of Flight
 */
#include "Pins.h"

////////////////////////////////
/* Functions */
////////////////////////////////

// Returns the distance of the time of flight in mm's
uint16_t getTimeOfFlightReading() {
  VL53L0X_RangingMeasurementData_t measure;

  timeOfFlight.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  bool checkingIncorrectData = (measure.RangeStatus != 4); // phase failures have incorrect data
  return checkingIncorrectData ? measure.RangeMilliMeter : 0;
}

// Prints out the time of flight reading
void testTimeOfFlight() {
  uint16_t distance = getTimeOfFlightReading();
  if(distance == 0) {
    Serial.println("Out of Range.");
  } else { 
    Serial.print("Distance (mm): "); Serial.println(distance);
  }
}
