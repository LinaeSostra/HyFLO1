/* Pins */
#ifndef PINS_H
#define PINS_H

// Time of Flight
const int sdaPin = 2;
const int sclPin = 3;

// Ultrasonic Sensor
const int triggerPin = 10;
const int echoPin = 9;

// Stepper Motor Easy Driver
const int stepperPin = 4; // Rising edge (LOW -> HIGH) triggers a step
const int directionPin = 5; // Set LOW to step 'forward', Set HIGH to step 'backwards'
const int enablePin  = 6; // Controls whether GND is enabled

// Tactile Position Switches
// These tactile switches are HIGH when not pressed, and LOW when pressed
const int homePin = 7;
const int endPin = 8;

// Pump
const int pumpPin = 11;

#endif
