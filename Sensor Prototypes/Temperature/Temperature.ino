// demo of Grove - High Temperature Sensor
// Thmc -> A5
// RoomTemp -> A4


#include "High_Temp.h"


void setup()
{
	Serial.begin(115200);
	Serial.println("grove - hight temperature sensor test demo");
}

void loop()
{
	HighTemp ht(A4, A5);
	ht.begin();
	Serial.print("Temp: ");
	Serial.println(ht.getThmc());
	delay(100);
}