/*
 Reports values from a K-series sensor back to the computer
 written by Jason Berger
 Co2Meter.com
*/
#include "kSeries.h" //include kSeries Library
#include <math.h>
#include <DHT.h>

#define SensitivityCode 43.11
#define TIAGain 499
#define DHT_PIN    7
int ozoneSetup();
int ozoneLoop();

kSeries K_30(8,9); //Initialize a kSeries Sensor with pin 12 as Rx and 13 as Tx


int Gas =A5;
int Temp =A4;
double GasCalc;
double GasConcentration;
double IntialGas;
double CalibrationFactor;

DHT dht(DHT_PIN, DHT11);


void setup()
{
 Serial.begin(9600); //start a serial port to communicate with the computer
 Serial.println("   AN-216  Example 2:  uses the kSeries.h library");
 ozoneSetup();
}
void loop()
{
 double co2 = K_30.getCO2('p'); //returns co2 value in ppm ('p') or percent ('%')
 double Temperature= K_30.getTemp('f');
 ozoneLoop();

 Serial.print("Co2 ppm = ");
 Serial.println(co2); //print value
 Serial.print("Ozone");
 Serial.println(GasCalc);
 

 Serial.print("Temperature ");
 Serial.println(Temperature); //print value
 delay(1500); //wait 1.5 seconds

}
 int ozoneSetup(){ Serial.begin(9600);
  double IntialGas;
  int CalibrationFactor;
  CalibrationFactor= SensitivityCode*TIAGain*0.000001;
  
  IntialGas= analogRead(Gas)*0.0048876;
  IntialGas= IntialGas*(1/CalibrationFactor);
  return 0;
}
int ozoneLoop(){for (int i = 0; i < 49; i++){
  Gas= Gas +analogRead(Gas);}
  Gas= Gas*0.02;
  double InitialGas;
  GasCalc= (Gas*0.0048876)- InitialGas;
  

  
  Serial.print(",");
  delay(1500);
  return 0;
}
float getTemperatureC() {
  return dht.readTemperature();
}

float getTemperatureF() {
  float fahrenheit = (dht.readTemperature() * 1.8) + 32;
  return fahrenheit;
}

float getHumidity() {
  return dht.readHumidity();
}


