
#include "src/SparkFun_VL6180X.h"

#define DEBUG // comment this line to disable debug (Serial Prints)

// Time of Flight Sensor
#define TIME_OF_FLIGHT_ADDRESS 0x29
#define TIME_OF_FLIGHT_MAX_DISTANCE 210 // mm

VL6180x sensor(TIME_OF_FLIGHT_ADDRESS);

// Ultrasonic Sensor
/*
 * The ultrasonic sensor requires 2 pins (http://wiki.jmoon.co/hcsr04/):
 * Trigger Pin is used to send out an ultrasonic high level pulse for at least 10 microseconds
 * Echo Pin automatically detects the returning pulse, measured in microseconds
 */
#define triggerPin 10
#define echoPin 9

#define SPEED_OF_SOUND 0.343 // mm per microsecond
#define DETECTION_THRESHOLD 100 // mm
#define MAX_ERROR_PERCENTAGE 0.1 // % (unitless)

#define TRIGGER_SWITCH_WAITTIME 2 // microseconds
#define TRIGGER_PULSE_WAITTIME 10 // microseconds
#define CONTAINER_DEBOUNCE_WAITTIME 1000 // milliseconds

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

  // Initialize Ultrasonic Sensor
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize Pump
  pinMode(pumpPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  // Check if there's a container present
  bool isContainerThere = checkForContainer();

  bool isReadyToDispenseLiquid = isContainerThere && isScanComplete && isNozzleCentered && !hasFinishedDispensing;
  while(isReadyToDispenseLiquid) {
    //analogWrite(pumpPin, 255);
    delay(1000); // BE SUPER CAREFUL WITH THIS!!!
    // TODO(Rebecca): Add Half Fill Functionality
    analogWrite(pumpPin, 0);
    delay(2000);
    hasFinishedDispensing = true;
    break;
  }
}

// Returns the time of flight reading as a height
int getTimeOfFlightReading() {
  int height = TIME_OF_FLIGHT_MAX_DISTANCE - sensor.getDistance();
  //assert(height >= 0);
  return height;
}

// Returns the ultrasonic distance reading as a distance
int getUltrasonicReading() {
  digitalWrite(triggerPin, LOW); 
  delayMicroseconds(TRIGGER_SWITCH_WAITTIME); // Waiting to update to LOW
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(TRIGGER_PULSE_WAITTIME); // Waiting for return signal
  digitalWrite(triggerPin, LOW);

  // Converting ultrasonic reading to distance
  //
  // Note: To convert the ultrasonic measurement from time (microseconds) to distance (mm),
  // the time for the wave to return needs to be halved as it presents both the time to hit
  // the object and return back to the sensor. Then this time needs to multipled by 
  // the speed of sound. All of this is calculated as a float, and then casted into an int
  // to prevent integer overflow/negative distance readings.
  //
  // distance = ((Time for wave to return / 2) * Speed of Sound)
  int distance = (int) (pulseIn(echoPin, HIGH) / 2 * SPEED_OF_SOUND);
  return distance;
}

// Checks whether an object has been placed in the vicinity or not
bool checkForContainer() {
  bool isContainerThere = false;
  int ultrasonicDistance = getUltrasonicReading();
  int temp = getUltrasonicReading();
#ifdef DEBUG
  Serial.print("Ultrasonic Distance: "); Serial.println(ultrasonicDistance);
  Serial.print("Temp Distance: "); Serial.println(temp);
#endif
  
  bool isObjectPresent = ultrasonicDistance < DETECTION_THRESHOLD;
  if (isObjectPresent) {
    // Debounce distance checking
    delay(CONTAINER_DEBOUNCE_WAITTIME);
    int ultrasonicDistance2 = getUltrasonicReading();
    bool isDistancePositive = ultrasonicDistance > 2;
    double errorPercentage = isDistancePositive ? (abs(ultrasonicDistance - ultrasonicDistance2) / double(ultrasonicDistance)) : 0; 
    
    // Checking if distance has stabilized
    bool hasDistanceStabilized = errorPercentage < MAX_ERROR_PERCENTAGE;
    if (hasDistanceStabilized) {
      isContainerThere = true;
    }
  }
  return isContainerThere;
}
