/*
 * Written By: Rebecca Dun
 * 
 * The purpose of this script is to test the HyFLO 1 dispensing
 * of liquid using Fuzzy Logic assuming the nozzle is in centered over 
 * the cup already.
 * 
 * This is tested on an UNO, and not the intended 4udino (Leonardo).
 */

#include "src/SparkFun_VL6180X.h"
#include "src/NewPing.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Time of Flight Sensor
#define TIME_OF_FLIGHT_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 210 // mm

VL6180x sensor(TIME_OF_FLIGHT_ADDRESS);

// Ultrasonic Sensor
/*
 * The ultrasonic sensor requires 2 pins (http://playground.arduino.cc/Code/NewPing):
 * Trigger Pin is used to send out an ultrasonic high level pulse for at least 10 microseconds
 * Echo Pin automatically detects the returning pulse, measured in microseconds
 * 
 * By default, the max distance the ultrasonic sensor reads 500 cm.
 */
#define triggerPin 10
#define echoPin 9

NewPing ultrasonicSensor(triggerPin, echoPin);

const unsigned int DETECTION_THRESHOLD = 15; // cm
const unsigned int MAX_CHANGE_IN_DISTANCE = 3; // cm
const unsigned int CONTAINER_DEBOUNCE_WAITTIME = 1000; // milliseconds

// Pump
#define pumpPin 11

// Buad Rate = Data Rate in Bits per Second
// Recommended Rate: 300, 600, 1200, 2400, 4800, 9600, 14400, 
// 19200, 28800, 38400, 57600, or 115200
#define BAUD_RATE 9600 

int rimHeight, rimHeight2 = 0; //TODO(Rebecca): Change this to dummy value.

bool isNozzleCentered = true;
bool isScanComplete = true;
bool hasFinishedDispensing = false;

void setup() {
  Serial.begin(BAUD_RATE);

  // Initialize Time of Flight Sensor
  if(sensor.VL6180xInit() != 0) {
    Serial.println("FAILED TO INITIALIZE");
  }
  sensor.VL6180xDefautSettings();

  delay(1000);

  // Initialize Pump
  pinMode(pumpPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  // Check if there's a container present
  bool isContainerThere = checkForContainer();

  /*bool isReadyToDispenseLiquid = isContainerThere && isScanComplete && isNozzleCentered && !hasFinishedDispensing;
  while(isReadyToDispenseLiquid) {
    //analogWrite(pumpPin, 255);
    delay(1000); // BE SUPER CAREFUL WITH THIS!!!
    // TODO(Rebecca): Add Half Fill Functionality
    analogWrite(pumpPin, 0);
    delay(2000);
    hasFinishedDispensing = true;
    break;
  }*/
}

// Returns the time of flight reading as a height
int getTimeOfFlightReading() {
  int height = TIME_OF_FLIGHT_MAX_DISTANCE - sensor.getDistance();
  //assert(height >= 0);
  return height;
}

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
