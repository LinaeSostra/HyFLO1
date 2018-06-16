// Returns the ultrasonic sensor distance reading in cm
unsigned int getUltrasonicReading() {
  return ultrasonicSensor.ping_cm();
}

// Checks whether an object has been placed in the vicinity or not
bool checkForContainer() {
  bool isContainerThere = false;
  unsigned int ultrasonicDistance = getUltrasonicReading();
  
#ifdef DEBUG
  Serial.print("Ultrasonic Distance: "); Serial.println(ultrasonicDistance);
#endif
  
  bool isObjectPresent = ultrasonicDistance <= DETECTION_THRESHOLD;
  if (isObjectPresent) {
    // Debounce distance checking
    delay(CONTAINER_DEBOUNCE_WAITTIME); 
    unsigned int ultrasonicDistance2 = getUltrasonicReading();
    unsigned int distanceChange = abs(ultrasonicDistance - ultrasonicDistance2);

#ifdef DEBUG
  Serial.print("Ultrasonic Distance2: "); Serial.println(ultrasonicDistance2);
#endif
    
    // Checking if distance has stabilized
    bool hasDistanceStabilized = distanceChange < MAX_CHANGE_IN_DISTANCE;
    if (hasDistanceStabilized) {
      isContainerThere = true; 
    }
  }
  return isContainerThere;
}
