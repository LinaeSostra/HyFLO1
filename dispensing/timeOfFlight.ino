 /*
 * Time of Flight
 */

// Initialize Time of Flight
Adafruit_VL53L0X timeOfFlight = Adafruit_VL53L0X();

// Global Constants
const int TIME_OF_FLIGHT_MAX_DISTANCE = 255; // mm
const int LIQUID_OFFSET = 20; // mm

// Checks the time of flight boots as intended
void timeOfFlightSetup() {
  Serial.println("Adafruit VL53L0X test");
  if(!timeOfFlight.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }

  Serial.println("Adafruit VL53L0X Booted Up Successfully");
}
//TODO(Rebecca): REMOVE!!
int counter = 0;
// Returns the distance of the time of flight in mm's
int getTimeOfFlightReading() {
  counter++;
  return counter;//25+LIQUID_OFFSET;
  /*
  VL53L0X_RangingMeasurementData_t measure;
  
  //Note: This should be false, but then readings aren't returned :/
  timeOfFlight.rangingTest(&measure, true);
  int distance = TIME_OF_FLIGHT_MAX_DISTANCE - measure.RangeMilliMeter;
  #ifdef DEBUG
    Serial.print("Time of Flight Readings: "); Serial.println(distance);
  #endif
  // Checking if distance is not overflowing
  bool reasonableDistance = distance < TIME_OF_FLIGHT_MAX_DISTANCE;

  return reasonableDistance ? distance : 0;
  */
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
