#include <SparkFun_VL6180X.h>
#define VL6180X_ADDRESS 0x29

VL6180x sensor(VL6180X_ADDRESS);

// Ultrasonic Parameters
#define trigPin 10
#define echoPin 9

// Tactile Switch Paramets
#define tactileHomePin 7
#define tactileEndPin 8

// Stepper Motor Easy Driver Parameters
#define stp 2
#define dir 3
#define EN  6

long duration, ultrasonicDistance;

#define dataAvg 15    // the number of data to average
int data[dataAvg];      // create vector called data 
int dataIndex = 0;              // the index of the current data reading
int total = 0;                  // the running total
int average = 0;                // the average

int8_t endPos;
int8_t homePos;


bool cupPlacementDelayFlag = false;   // for cup placement delay
bool scanComplete = false; // true = scan complete

void setup() {
  Serial.begin(9600); //Start Serial at 115200bps
  
  sensor.VL6180xDefautSettings(); //Load default settings to get started.
  sensor.VL6180xInit();

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  
  digitalWrite(dir, HIGH); //Pull direction pin HIGH to move "reverse" (back to home)

  pinMode (tactileHomePin, INPUT);
  pinMode (tactileEndPin, INPUT);

  // For running average algorithm. 
  for (int thisReading = 0; thisReading < dataAvg; thisReading++) {
    data[thisReading] = 0;
  }

  // Check current state of system
  homePos = digitalRead(tactileHomePin);
  endPos = digitalRead(tactileEndPin);
}

// MAIN LOOP /**********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void loop() {

  //check if the system is at Home Position. If not, fix it. 
  if (homePos == LOW) {
    returnHome();
  }

  // check if there's a cup
  checkProximity();
  //Serial.println(ultrasonicDistance);
  
  // cup is placed, so start prelim. scan. 
  while (ultrasonicDistance < 10 && !scanComplete) {
    if (!cupDisplacementFlag){
      delay(2000);
      cupDisplacementFlag = true;
    }
    StepForward();
    //Check if scan is complete
    endPos = digitalRead(tactileEndPin);
    if (endPos == HIGH) {
      scanComplete = true;
      homePos = digitalRead(tactileHomePin);
      returnHome();
      break;
    }
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void StepForward() {
  //Serial.println("Moving forward at default step mode.");
 
  for (int i = 0; i < 100; i++) {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(70); //2000 was best
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(70); //2000 was best
  }

  uint8_t ToF_distance = sensor.getDistance();

  total = total - data[dataIndex];
  data[dataIndex] = ToF_distance;
  total = total + data[dataIndex];
  dataIndex = dataIndex + 1;

  if (dataIndex >= dataAvg) { // if we're at the end of the array...
    dataIndex = 0;
  }

  average = total / dataAvg; 

  Serial.print("Distance = "); Serial.println(average);
}



// Reverse default microstep mode function
void StepReverse() {
  //Serial.println("Moving reverse at default step mode.");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"

  for (int i = 0; i < 100; i++) {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(70); //2000 was best
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(70); //2000 was best
  }
}



void returnHome() {
  while (homePos == LOW) {
    digitalWrite(dir, HIGH); // Reverse direction
    digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
    StepReverse();
    homePos = digitalRead(tactileHomePin);
    if (homePos == HIGH) {
      digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
      digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
      break;
    }
  }
}


void checkProximity() {
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  ultrasonicDistance = (duration / 2) / 29.1;

  if (ultrasonicDistance < 10) {
    delay(100);
    digitalWrite(trigPin, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    ultrasonicDistance = (duration / 2) / 29.1;
  }
}

// Reset Easy Driver pins to default states
void resetEDPins() {
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(EN, HIGH);
}
