/*
 * Time of Flight
 */

// Initialize Time of Flight
Adafruit_VL53L0X timeOfFlight = Adafruit_VL53L0X();

// Global Constants
const int TIME_OF_FLIGHT_MAX_DISTANCE = 400; // mm

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
int getTimeOfFlightReading() {
  VL53L0X_RangingMeasurementData_t measure;

  timeOfFlight.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
#ifdef DEBUG
  Serial.print("Straight Time of Flight Value: ");Serial.println(measure.RangeMilliMeter);
#endif
  bool inGoodPhase = (measure.RangeStatus != 4); // phase failures have incorrect data
  int distance = TIME_OF_FLIGHT_MAX_DISTANCE - measure.RangeMilliMeter;
  
  bool isDataNotOverflowing = distance <= TIME_OF_FLIGHT_MAX_DISTANCE && distance >= 0;
  bool isDataCorrect = inGoodPhase && isDataNotOverflowing;
  
  return isDataCorrect ? distance : 0;
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
