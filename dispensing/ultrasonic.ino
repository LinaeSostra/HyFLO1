/*
 * Ultrasonic Sensor
 * 
 * The ultrasonic sensor requires 2 pins (http://playground.arduino.cc/Code/NewPing):
 * Trigger Pin is used to send out an ultrasonic high level pulse for at least 10 microseconds
 * Echo Pin automatically detects the returning pulse, measured in microseconds
 * 
 * By default, the max distance the ultrasonic sensor reads 500 cm.
 */

// Initialize ultrasonic sensor
NewPing ultrasonicSensor(triggerPin, echoPin);

////////////////////////////////
/* Global Constants */
////////////////////////////////
const unsigned int DETECTION_THRESHOLD = 15; // cm
const unsigned int MAX_CHANGE_IN_DISTANCE = 3; // cm
const unsigned int CONTAINER_DEBOUNCE_WAITTIME = 1000; // milliseconds

////////////////////////////////
/* Functions */
////////////////////////////////

// Returns the ultrasonic sensor distance reading in cm
unsigned int getUltrasonicReading() {
  return ultrasonicSensor.ping_cm();
}

// Checks if the two ultrasonic distances have stopped changing (stabilized)
bool hasDistanceStabilized(unsigned int distance, unsigned int distance2) {
  int distanceChange = abs(distance - distance2);
  return distanceChange < MAX_CHANGE_IN_DISTANCE;
}

// Checks whether an object has been placed in the vicinity or not
bool checkForContainer() {
  // Check if distance within detection area
  unsigned int distance = getUltrasonicReading();
  bool isObjectPresent = distance <= DETECTION_THRESHOLD;
  if (!isObjectPresent) { return false; }
  
  // Debounce checking
  delay(CONTAINER_DEBOUNCE_WAITTIME); 
  unsigned int distance2 = getUltrasonicReading();
  #ifdef DEBUG
    Serial.print("Ultrasonic Distance: "); Serial.println(distance);
    Serial.print("Ultrasonic Distance2: "); Serial.println(distance2);
  #endif
    
  // Checking if distance has stabilized
  bool isContainerThere = hasDistanceStabilized(distance, distance2);
  return isContainerThere;
}
