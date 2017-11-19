/* Wildfire Group / FAU 2017 */

#include "kSeries.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <math.h>
#include <DHT.h>

/* LORA */
#define RF95_FREQ 915.0
#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3

/* CONSTANTS */
#define SensitivityCode 43.11
#define TIAGain 499
#define NUM_SAMPLES 10

/* PINS */
#define DHT_PIN 7
#define CO2TX_PIN 8
#define CO2RX_PIN 9
#define FAN_PIN A2
#define PPM_PIN A3
#define PPM_LED 10
#define SEND_LED A1

struct WildfirePacket {
	long  loop_count;
	float co2;
	float co2_temp;
	float ozone_gas;
	float temp;
	float ppm;
	float battery;
};

int      ozone_setup();	// do not thing this has an impact
float    get_ozone_gas();
float    get_co2_ppm();
int      get_co2_temp();
float    get_temperature_c();
float    get_temperature_f();
float    get_humidity();
float    get_ppm();
float    get_battery_level();
int      lora_setup();
int		 lora_send(char* message, int size);
uint8_t* lora_receive();
int	     print_packet(WildfirePacket *packet);

RH_RF95 rf95(RFM95_CS, RFM95_INT);	// Lora
kSeries K_30(CO2TX_PIN, CO2RX_PIN);	// CO2
DHT dht(DHT_PIN, DHT11);			// Temperature

void setup() {
	Serial.begin(9600);
	Serial.println("Wildfire Project Starting Up....");
	delay(1000);
	pinMode(A0, INPUT);
	pinMode(A1, OUTPUT);
	pinMode(FAN_PIN, OUTPUT);
	digitalWrite(FAN_PIN, HIGH);
	ozone_setup();
	lora_setup();
}

int loopcount = 0;

void loop() {
	bool packet_sent = 0;
	char buffer[28] = "";
	uint8_t* buf;
	struct WildfirePacket *packet = (WildfirePacket*)malloc(sizeof(WildfirePacket));

	memset(packet, 0, sizeof(WildfirePacket));
	loopcount = loopcount + 1;
	digitalWrite(FAN_PIN, HIGH);

	packet->loop_count = loopcount;
	packet->temp       = get_temperature_f();
	packet->co2        = get_co2_ppm();
	packet->co2_temp   = get_co2_temp();
	packet->ozone_gas  = get_ozone_gas();
	packet->ppm        = get_ppm();
	packet->battery    = get_battery_level();

	memcpy(buffer, &packet->loop_count, sizeof(long));
	memcpy(buffer + sizeof(long), &packet->temp, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 1), &packet->co2, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 2), &packet->co2, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 3), &packet->ozone_gas, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 4), &packet->ppm, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 5), &packet->battery, sizeof(float));

	print_packet(packet);

	while (!packet_sent) {
		// Send the packet
		lora_send(buffer, 28);

		// Receive response
		buf = lora_receive();
		delay(100);

		// Check response matches what we sent
		if (memcmp(buf, buffer, 28) == 0) {
			Serial.println("Packets match. Sending ACK.");
			lora_send("ACK", 4);
			packet_sent = 1;
		} else {
			Serial.println("ERROR: PACKETS DO NOT MATCH! SENDING AGAIN...");
			delay(500);
		}
	}

	free(packet);
	//delay(1000);
	digitalWrite(FAN_PIN, LOW);
	//delay(1000);
}

int ozone_setup() {
	float initial_gas;
	float calibration_factor;

	calibration_factor = SensitivityCode * TIAGain * 0.000001;

	initial_gas = analogRead(A5) * 0.0048876;
	initial_gas = initial_gas * (1 / calibration_factor);
	return 0;
}

float get_ozone_gas() {
	double Gas;

	for (int i = 0; i < 49; i++) {
		Gas = Gas + analogRead(Gas);
	}
	Gas = Gas * 0.02;

	return (Gas * 0.0048876) - 0;
}

float get_co2_ppm() {
	return K_30.getCO2('p');
}

int get_co2_temp() {
	return K_30.getTemp('f');
}

float get_temperature_c() {
	return dht.readTemperature();
}

float get_temperature_f() {
	float fahrenheit = (dht.readTemperature() * 1.8) + 32;
	return fahrenheit;
}

float get_humidity() {
	return dht.readHumidity();
}

float get_ppm() {
	float voMeasured = 0;
	float calcVoltage = 0;
	float dustDensity = 0;
	float total = 0;

	for (int i = 0; i < 19; i++) {
		digitalWrite(PPM_LED, LOW);			// power on the LED
		delayMicroseconds(280);				// sample time
		voMeasured = analogRead(PPM_PIN);	// read the dust value
		delayMicroseconds(40);				// delta time
		digitalWrite(PPM_LED, HIGH);		// turn the LED off
		delayMicroseconds(9680);
		calcVoltage = voMeasured * (5.0 / 1024);
		delayMicroseconds(9680);
		dustDensity = (0.17 * calcVoltage - 0.1) * 1000; // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/ Chris Nafis (c) 2012
		total += dustDensity;
	}
	dustDensity = total / 20;

	return dustDensity;
}


float get_battery_level() {
	float voltage = 0;  // raw voltage
	float voltage_actual = 0; // voltage calculated using known voltage divider circuit
	float battery_level = 0;  // percentage of battery used before cut off voltage
	delay(10);

	voltage = (analogRead(A0) / 1024.0)*5.015;
	voltage_actual = (voltage * 8) / 5.015;
	battery_level = (-53.347*voltage_actual) + 439.76;
	if (voltage_actual < 6) {
		// Serial.println("Your Battery Level is Low!");
	}

	return battery_level;
}

int lora_setup() {
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);

	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);

	while (!rf95.init()) {
		Serial.println("LoRa radio init failed"); delay(100);
	}
	Serial.println("LoRa radio init OK!");

	if (!rf95.setFrequency(RF95_FREQ)) {
		Serial.println("setFrequency failed"); delay(100);
	}
	Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

	rf95.setTxPower(15, false);	// Transmitter powers from 5 to 23 dBm:
	rf95.setThisAddress(2);
	rf95.setHeaderFrom(2);
	rf95.setHeaderTo(1);
	return 0;
}

int lora_send(char* message, int size) {
	analogWrite(SEND_LED, 255);
	rf95.send((uint8_t *)message, size);
	rf95.waitPacketSent();
	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(buf);

	analogWrite(SEND_LED, 0);
	return 0;
}

uint8_t* lora_receive() {
	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(buf);

	if (rf95.waitAvailableTimeout(5000)) {
		if (rf95.recv(buf, &len)) {
			Serial.print("Got reply, RSSI: "); Serial.println(rf95.lastRssi(), DEC);
		} else {
			Serial.println("Receive failed...");
			return NULL;
		}
	} else {
		Serial.println("Timed out...");
		return NULL;
	}
	return buf;
}

int print_packet(WildfirePacket *packet){
	char buf[250];
	sprintf(buf, "Packet[%d] =>", packet->loop_count);
	Serial.print(buf);
	Serial.print(" Temp: "); Serial.print(packet->temp);
	Serial.print(", Co2: "); Serial.print(packet->co2);
	Serial.print(", Co2_Temp: "); Serial.print(packet->co2_temp);
	Serial.print(", Ozone_Gas: "); Serial.print(packet->ozone_gas);
	Serial.print(", PPM: "); Serial.print(packet->ppm);
	Serial.print(", Battery: "); Serial.println(packet->battery);
	return 0;
}
