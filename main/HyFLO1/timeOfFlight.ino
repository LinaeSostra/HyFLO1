/*
 * Time of Flight
 */

// Initialize Time of Flight
#define TIME_OF_FLIGHT_ADDRESS 0x29
VL6180x sensor(TIME_OF_FLIGHT_ADDRESS);

// Global Constants
const int TIME_OF_FLIGHT_MAX_DISTANCE = 255; // mm

// Rolling Average Smoothing Variables
const int MAX_SAMPLES = 5;     // the number of readings to average
int readings[MAX_SAMPLES];
int readIndex = 0;              // the index of the current reading
int averageHeight = 0;          // the average

// Checks the time of flight boots as intended
void timeOfFlightSetup() {
  #ifdef DEBUG
  Serial.println("Sparkfun VL6180X test");
  #endif
  if(sensor.VL6180xInit() != 0) {
    Serial.println("FAILED TO INITIALIZE VL6180X");
  }
  sensor.VL6180xDefautSettings();

  // Initializing readings array to 0s. (for running average algorithm) 
  for (int thisReading = 0; thisReading < MAX_SAMPLES; thisReading++) {
    readings[thisReading] = 0;
  }
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

void smoothReading() {
  int height = getTimeOfFlightReading();

  readings[readIndex] = height;
  int total = getRunningTotal();
  readIndex = (readIndex + 1) % MAX_SAMPLES;
  averageHeight = total / MAX_SAMPLES;

#ifdef DEBUG
  Serial.print("Time of Flight Height = "); Serial.println(averageHeight);
#endif
}

int getRunningTotal() {
  int total = 0;
  for(int i = 0; i < MAX_SAMPLES; i++) {
    total = total + readings[i];
  }
  return total;
}

int getAverageHeight() {
  return averageHeight;
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
