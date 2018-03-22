#include <SparkFun_VL6180X.h>
#define VL6180X_ADDRESS 0x29

VL6180x sensor(VL6180X_ADDRESS);

// for Ultrasonic
#define trigPin 10
#define echoPin 9

// for Stepper Motor Easy Driver 
#define stp 2
#define dir 3
#define EN  6

long duration, US_distance;   // US_distance = ultrasonic distance

const int numReadings = 15;     // the number of readings to average
int readings[numReadings];      // create vector called readings 
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int8_t position2;   // end position
int8_t position1;   // home position
int x = 0;   // for cup placement delay

int scanComplete = 0; // 1 = scan complete

void setup() {
  Serial.begin(9600); //Start Serial at 115200bps
  sensor.VL6180xDefautSettings(); //Load default settings to get started.
  sensor.VL6180xInit();

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(dir, HIGH); //Pull direction pin HIGH to move "reverse" (back to home)

  pinMode (7, INPUT); //tactile switch Home Position
  pinMode (8, INPUT); //tactile switch End Position

  // for running average algorithm. 
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  //check current state of system
  position1 = digitalRead(7); // Home Position
  position2 = digitalRead(8); // End Position
}

// MAIN LOOP /**********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void loop() {

  //check if the system is at Home Position. If not, fix it. 
  if (position1 == LOW) {
    returnHome();
  }

  // check if there's a cup
  checkProximity();
  //Serial.println(US_distance);
  
  // cup is placed, so start prelim. scan. 
  while (US_distance < 10 && scanComplete == 0) {
    if (x == 0){
      delay(2000);
      x = 1;
    }
    StepForward();
    //Check if scan is complete
    position2 = digitalRead(8);
    if (position2 == HIGH) {
      scanComplete = 1;
      position1 = digitalRead(7);
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

  total = total - readings[readIndex];
  readings[readIndex] = ToF_distance;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0;
  }

  average = total / numReadings; 

  Serial.print("Distance = "); Serial.println(average);
}



//Reverse default microstep mode function
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
  while (position1 == LOW) {
    digitalWrite(dir, HIGH); // Reverse direction
    digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
    StepReverse();
    position1 = digitalRead(7);
    if (position1 == HIGH) {
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
  US_distance = (duration / 2) / 29.1;

  if (US_distance < 10) {
    delay(100);
    digitalWrite(trigPin, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    US_distance = (duration / 2) / 29.1;
  }
}

//Reset Easy Driver pins to default states
void resetEDPins() {
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(EN, HIGH);
}


