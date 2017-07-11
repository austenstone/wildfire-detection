/*
Name:		WildfireSensor.ino
Created:	6/30/2017 1:57:25 PM
Author:	Austen
*/

#include <SoftwareSerial.h>
#include <Time.h>
#include <DHT.h>
/************************Hardware Related Macros************************************/
#define         RL_VALUE                     (5)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.83)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
//which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
//cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
/*****************************Globals***********************************************/
float           LPGCurve[3] = { 2.3,0.21,-0.47 };   //two points are taken from the curve. 
													//with these two points, a line is formed which is "approximately equivalent"
													//to the original curve. 
													//data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3] = { 2.3,0.72,-0.34 };    //two points are taken from the curve. 
													//with these two points, a line is formed which is "approximately equivalent" 
													//to the original curve.
													//data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] = { 2.3,0.53,-0.44 }; //two points are taken from the curve. 
													//with these two points, a line is formed which is "approximately equivalent" 
													//to the original curve.
													//data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro = 10;							//Ro is initialized to 10 kilo ohms

int x = 0;


//LED Connections
#define LED_RED		10
//Digital Connections
#define DHT_PIN		2
#define SERVO_PIN	6
#define MOTION_PIN	8
#define FAN_PIN		9
#define PPM_LED		11
//Analog Connections
#define SMOKE_PIN	A4
#define PPM_PIN		A5
#define O3_PIN		A3
#define IR_PIN		A3
//Serial Connections
#define MH_Z19_RX	5
#define MH_Z19_TX	6

DHT dht(DHT_PIN, DHT11);

SoftwareSerial co2Serial(MH_Z19_RX, MH_Z19_TX);

//Servo servo;

void setup() {
	Serial.println("Starting up...");	// let sensor heat up

										//Start Serials
	Serial.begin(9600);
	co2Serial.begin(9600);				// 9600 baudrate for MH_Z19
	while (!Serial);					// check serial connection

										//PinModes
	pinMode(LED_RED, OUTPUT);
	pinMode(PPM_LED, OUTPUT);
	pinMode(MOTION_PIN, INPUT);
	pinMode(MOTION_PIN, INPUT);

	//Ro = MQCalibration(SMOKE_PIN);      // Calibrating the sensor. Please make sure the sensor is in clean air

	//servo.attach(SERVO_PIN);
}

void loop() {
	float o3 = getO3();
	float temp = getTemperatureF();
	float humidity = getHumidity();
	int lpg = getGas(MQRead(SMOKE_PIN) / Ro, "LPG");
	int concentration = getGas(MQRead(SMOKE_PIN) / Ro, "CO");
	int smoke = getGas(MQRead(SMOKE_PIN) / Ro, "SMOKE");
	float ppm = getPPM();

	Serial.print("1. temp/humidity:       ");
	Serial.print(temp);
	Serial.print(", ");
	Serial.println(humidity);
	Serial.print("2. Density:             ");
	Serial.println(ppm);
	Serial.print("3. Smoke LPG/Co/Smoke:  ");
	Serial.print(lpg);
	Serial.print(", ");
	Serial.print(concentration);
	Serial.print(", ");
	Serial.println(smoke);
	Serial.print("4. O3 Detector:         ");
	Serial.println(o3);
	Serial.println("");
	if (smoke > 300) {
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else {
		digitalWrite(LED_BUILTIN, LOW);
	}

	delay(1000);
}

boolean getMovement() {
	return digitalRead(MOTION_PIN);;
}

int readCO2() {
	byte cmd[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 }; // command to ask for data
	byte response[9];														// for answer
	memset(response, 0, 9);													// set entire buffer to 0
	printCO2(cmd, "Command:\t");						// print command

	if (co2Serial.available() > 0) {
		co2Serial.write(cmd, 9);						// write command
		co2Serial.readBytes(response, 9);				// read response
		printCO2(response, "Response:\t");				// print response
	}
	else {
		Serial.println("CO2 serial not avaliable");
	}
	Serial.println(" ");

	//calculations
	int responseHigh = (int)response[2];
	int responseLow = (int)response[3];
	int ppm = (256 * responseHigh) + responseLow;
	return ppm;											// return ppm
}

float getPPM() {
	float voMeasured = 0;
	float calcVoltage = 0;
	float dustDensity = 0;
	float Total = 0;

	for (int i = 0; i < 19; i++) {
		digitalWrite(PPM_LED, LOW);			// power on the LED
		delayMicroseconds(280);				// sample time
		voMeasured = analogRead(PPM_PIN);	// read the dust value
		delayMicroseconds(40);				// delta time
		digitalWrite(PPM_LED, HIGH);		// turn the LED off
		delayMicroseconds(9680);
		calcVoltage = voMeasured * (5.0 / 1024);
		delayMicroseconds(9680);
		dustDensity = (0.17 * calcVoltage - 0.1) * 1000; // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/  Chris Nafis (c) 2012
		Total += dustDensity;
	}
	dustDensity = Total / 20;

	return dustDensity;
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

void printCO2(byte buffer[9], String intro) {
	char format[128];
	Serial.print(intro);
	for (int i = 0; i < 9; i++) {
		sprintf(format, "0x%02X", buffer[i]);
		Serial.print(format);
		Serial.print(" ");
	}
	Serial.println(" ");
}

float getO3() {
	return analogRead(O3_PIN);
}

//void servoControl(int movment) {
//	servo.write(movment);
//}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
across the load resistor and its resistance, the resistance of the sensor
could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc) {
	return (((float)RL_VALUE*(1023 - raw_adc) / raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use
MQResistanceCalculation to calculates the sensor resistance in clean air
and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
10, which differs slightly between different sensors.
************************************************************************************/
float MQCalibration(int mq_pin) {
	int i;
	float val = 0;

	for (i = 0; i<CALIBARAION_SAMPLE_TIMES; i++) {            //take multiple samples
		val += MQResistanceCalculation(analogRead(mq_pin));
		delay(CALIBRATION_SAMPLE_INTERVAL);
	}
	val = val / CALIBARAION_SAMPLE_TIMES;					  //calculate the average value

	val = val / RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
															//according to the chart in the datasheet 

	return val;
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
The Rs changes as the sensor is in the different consentration of the target
gas. The sample times and the time interval between samples could be configured
by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin) {
	int i;
	float rs = 0;

	for (i = 0; i<READ_SAMPLE_TIMES; i++) {
		rs += MQResistanceCalculation(analogRead(mq_pin));
		delay(READ_SAMPLE_INTERVAL);
	}

	rs = rs / READ_SAMPLE_TIMES;

	return rs;
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which
calculates the ppm (parts per million) of the target gas.
************************************************************************************/
int getGas(float rs_ro_ratio, String command) {
	if (command == "LPG") {
		return MQGetPercentage(rs_ro_ratio, LPGCurve);
	}
	else if (command == "CO") {
		return MQGetPercentage(rs_ro_ratio, COCurve);
	}
	else if (command == "SMOKE") {
		return MQGetPercentage(rs_ro_ratio, SmokeCurve);
	}
	return 0;
}

/*****************************  MQGetPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
pcurve      - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
of the line could be derived if y(rs_ro_ratio) is provided. As it is a
logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
value.
************************************************************************************/
int  MQGetPercentage(float rs_ro_ratio, float *pcurve) {
	return (pow(10, (((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0])));
}