/*
 Name:		Ozone.ino
 Created:	9/23/2017 9:42:55 AM
 Author:	Austen
*/

#include <stdio.h>

#define SensitivityCode 25
#define TIAGain 499

#define VGASPIN  A5
#define VREFPIN  A4
#define VTEMPPIN A3

void setup() {
	Serial.println("Starting...");
	Serial.begin(9600);
	pinMode(VGASPIN, INPUT);
	pinMode(VREFPIN, INPUT);
	pinMode(VTEMPPIN, INPUT);
}

void loop() {
	gasConcentration(VGASPIN, VREFPIN, VTEMPPIN);
	delay(1000);
}

// Returns the concentration of gas.
int gasConcentration(uint32_t gasPin, uint32_t refPin, uint32_t tempPin) {
	double concentration;
	double calibrationFactor;
	int gas;
	int temp = analogRead(tempPin);

	calibrationFactor = SensitivityCode * TIAGain * .0000001;

	concentration = (1 / calibrationFactor) * (analogRead(gasPin) - (analogRead(refPin) - 0));

	char* message = (char*)malloc(50);
	sprintf(message, "calib: %f, conc: %f\n", calibrationFactor, concentration);
	Serial.print(message);
	free(message);
	return 0;
}