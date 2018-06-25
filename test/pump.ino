/*
 * Pump
 */

//Initialize Pump
 void pumpSetup() {
  pinMode(pumpPin, OUTPUT);
 }

 void pumpOn() {
  analogWrite(pumpPin, 255);
 }

 void pumpOff() {
  analogWrite(pumpPin, 0);
 }

// Turns the pump on for a second, then off.
 void testPump() {
  pumpOn();
  delay(1000);
  pumpOff();
 }
