/*
 * Time of Flight
 */

// Initialize Time of Flight
#define TIME_OF_FLIGHT_ADDRESS 0x29
VL6180x sensor(TIME_OF_FLIGHT_ADDRESS);

// Global Constants
const int TIME_OF_FLIGHT_MAX_DISTANCE = 255; // mm

// Checks the time of flight boots as intended
void timeOfFlightSetup() {
  Serial.println("Sparkfun VL6180X test");
  if(sensor.VL6180xInit() != 0) {
    Serial.println("FAILED TO INITIALIZE VL6180X");
  }
  sensor.VL6180xDefautSettings();
}

// Returns the distance of the time of flight in mm's
int getTimeOfFlightReading() {
  int height = TIME_OF_FLIGHT_MAX_DISTANCE - sensor.getDistance();
  //assert(height >=  0);
  return height;
}

// Plots out the time of flight reading for serial plotter
void plotTimeOfFlight() {
  int distance = getTimeOfFlightReading();
  Serial.println(distance);
}

// Prints out the time of flight reading for serial monitor
void testTimeOfFlight() {
  int distance = getTimeOfFlightReading();
  bool isObjectOutOfRange = distance == 0; 
  if(isObjectOutOfRange) {
    Serial.println("Out of Range.");
  } else { 
    Serial.print("Distance (mm): "); Serial.println(distance);
  }
}
