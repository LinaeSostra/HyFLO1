#include <SparkFun_VL6180X.h>
#define VL6180X_ADDRESS 0x29

VL6180x sensor(VL6180X_ADDRESS);

// for Ultrasonic
#define trigPin 10
#define echoPin 9

// for Stepper Motor Easy Driver 
#define stp 4
#define dir 5
#define EN  6

// Tactile Position Switches
#define homePin 7
#define endPin 8

long duration, US_distance;   // US_distance = ultrasonic distance

const int numReadings = 5;     // the number of readings to average
int readings[numReadings];      // create vector called readings 
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int8_t endPosition;
int8_t homePosition;
int x = 0;   // for cup placement delay

bool isScanComplete = false;

int rim1_Location;
int rim1_location_done = 0;
int rim1_AfterCounter = 0; 
int rim2_Location;

int rim1_Height = 0;
int rim2_Height = 0;

int nozzleCentered = 0; 

int stepCounter = 0; 

void setup() {
  Serial.begin(9600); //Start Serial at 115200bps
  sensor.VL6180xDefautSettings(); //Load default settings to get started.
  sensor.VL6180xInit();

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(dir, HIGH); //Pull direction pin HIGH to move "reverse" (back to home)

  pinMode (homePin, INPUT_PULLUP);
  pinMode (endPin, INPUT_PULLUP);

  // for running average algorithm. 
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  //check current state of system
  homePosition = digitalRead(homePin);
  endPosition = digitalRead(endPin);
}

// MAIN LOOP /**********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void loop() {

  //check if the system is at Home Position. If not, fix it. 
  if (homePosition == HIGH && !isScanComplete) {
    returnHome();
  }

  // check if there's a cup
  checkProximity();
  //Serial.println(US_distance);
  
  // cup is placed, so start prelim. scan. 
  while (US_distance < 15 && !isScanComplete) {
    if (x == 0){
      delay(2000);
      x = 1;
    }
    StepForward();
    //Check if scan is complete
    endPosition = digitalRead(endPin);
    if (endPosition == LOW) {
      isScanComplete = true;
      //Serial.print("Total Steps: "); Serial.println(stepCounter);
      homePosition = digitalRead(homePin);
      resetEDPins();
      break;
    }
  }
  while (US_distance < 10 && isScanComplete && nozzleCentered == 0){
    StepReverse();
    //Serial.print("Goto Step Counter = "); Serial.println(stepCounter);
    if (stepCounter == (rim1_Location+rim2_Location)/2){
      nozzleCentered = 1;
      resetEDPins();
      break;
    }
  }
}

//FUNCTIONS /***********************************************************************
////////////////////////////////////////////////////////////////////////////////////

void StepForward() {
  //Serial.println("Moving forward at default step mode.");
 
  for (int i = 0; i < 100; i++) {
    digitalWrite(stp, HIGH); // Trigger one step forward
    delayMicroseconds(70); // 2000 was best
    digitalWrite(stp, LOW); // Pull step pin low so it can be triggered again
    delayMicroseconds(70); // 2000 was best
  }

  uint8_t ToF_distance = sensor.getDistance();

  total = total - readings[readIndex];
  readings[readIndex] = ToF_distance;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0;
  }

  average = 207 - (total / numReadings); 

  Serial.print("Distance = "); Serial.println(average);
  stepCounter++;

  // Find first maxima
  if(average > rim1_Height && rim1_location_done == 0 && stepCounter > 30){
    rim1_Height = average;
    rim1_Location = stepCounter;
    //Serial.print("Rim 1 Location = "); Serial.println(rim1_Location);
  }
  
  if(average < rim1_Height && rim1_location_done == 0){
    rim1_AfterCounter++;
    //Serial.print("R1 AfterCounter = "); Serial.println(rim1_AfterCounter);
    if (rim1_AfterCounter == 120){
      rim1_location_done = 1;
      rim1_AfterCounter = 0;
      //Serial.println("Rim 1 Location"); Serial.println(rim1_Location);
    }
  }
  
  // Find second Maxima
  if(average > rim2_Height && stepCounter > 10 && stepCounter < 265 && rim1_location_done == 1){
    rim2_Height = average;
    rim2_Location = stepCounter;
    //Serial.print("Rim 2 Location"); Serial.println(rim2_Location);
  }

  
}
 







//Reverse default microstep mode function
void StepReverse() {
  //Serial.println("Moving reverse at default step mode.");
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
  digitalWrite(dir, HIGH); //Pull direction pin low to move "forward"
  for (int i = 0; i < 100; i++) {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(70); //2000 was best
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(70); //2000 was best
  }
  stepCounter--;
}



void returnHome() {
  while (homePosition == HIGH) {
    digitalWrite(dir, HIGH); // Reverse direction
    digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
    StepReverse();
    homePosition = digitalRead(homePin);
    if (homePosition == LOW) {
      digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
      digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
      stepCounter = 0; 
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

  if (US_distance < 15) {
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
