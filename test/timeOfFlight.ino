/*
 * Time of Flight
 */
#include "Pins.h"

// Initialize Time of Flight
Adafruit_VL53L0X timeOfFlight = Adafruit_VL53L0X();

////////////////////////////////
/* Global Constants */
////////////////////////////////
const int TIME_OF_FLIGHT_MAX_DISTANCE = 210; // mm

////////////////////////////////
/* Functions */
////////////////////////////////
// Checks the time of flight boots as intended
void timeOfFlightSetup() {
  Serial.println("Adafruit VL53L0X test");
  if(!timeOfFlight.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  Serial.println("Adafruit VL53L0X Booted Up Successfully");
}

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
  
  bool isObjectOutOfRange = distance == 0; 
  if(isObjectOutOfRange) {
    Serial.println("Out of Range.");
  } else { 
    Serial.print("Distance (mm): "); Serial.println(distance);
  }
}
