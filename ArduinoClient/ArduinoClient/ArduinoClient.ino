/* Wildfire Group / FAU 2017 */

#include "kSeries.h"
#include <SPI.h>
#include <RH_RF95.h>
#include <DHT.h>

/* LORA */
#define RF95_FREQ 915.0
#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3
#define NODE_ADDR 3

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
	char  chksum;
};

int ozone_setup();	// do not think this has an impact
float get_ozone_gas();
float get_co2_ppm();
int get_co2_temp();
float get_temperature_c();
float get_temperature_f();
float get_humidity();
float get_ppm();
float get_battery_level();
char chksum(char* buffer, int length);
int lora_setup();
int lora_send(char* message, int size);
uint8_t* lora_receive();
int process_command(uint8_t* buf);

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

int fan_on = 1;
int loopcount = 0;
unsigned long delayt = 5000; //100000;
unsigned long start = millis();

void loop() {
	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(buf);
	unsigned long now = millis();

	if ((now - start) >= delayt) {
		Serial.println("SENDING!");
		start = millis();
		send_sensor_data();
	}
	if (rf95.available()) {
		if (rf95.recv(buf, &len)) {
			Serial.print("RECEIVED: "); Serial.println((char*)buf);
			if (process_command(buf) != 0) {
				Serial.println("Commanding Processing Failed.");
			}
		}
	}
}

int send_sensor_data() {
	char buffer[29] = {};
	uint8_t* buf;
	struct WildfirePacket *packet = (WildfirePacket*)malloc(sizeof(WildfirePacket));
	int sent = 0;

	memset(packet, 0, sizeof(WildfirePacket));
	loopcount = loopcount + 1;
	if (fan_on) digitalWrite(FAN_PIN, HIGH);

	packet->loop_count = loopcount;
	memcpy(buffer, &packet->loop_count, sizeof(long));
	Serial.print("Packet["); Serial.print(packet->loop_count); Serial.print("] = ");
	packet->temp = get_temperature_f();
	memcpy(buffer + sizeof(long), &packet->temp, sizeof(float));
	Serial.print(" Tmp: "); Serial.print(packet->temp);
	packet->co2 = 12;// get_co2_ppm();
	packet->co2_temp = 14;// get_co2_temp();
	memcpy(buffer + sizeof(long) + (sizeof(float) * 1), &packet->co2, sizeof(float));
	memcpy(buffer + sizeof(long) + (sizeof(float) * 2), &packet->co2, sizeof(float));
	Serial.print(", Co2: "); Serial.print(packet->co2);
	Serial.print(", CoT: "); Serial.print(packet->co2_temp);
	packet->ozone_gas = get_ozone_gas();
	memcpy(buffer + sizeof(long) + (sizeof(float) * 3), &packet->ozone_gas, sizeof(float));
	Serial.print(", OzG: "); Serial.print(packet->ozone_gas);
	packet->ppm = get_ppm();
	memcpy(buffer + sizeof(long) + (sizeof(float) * 4), &packet->ppm, sizeof(float));
	Serial.print(", PPM: "); Serial.print(packet->ppm);
	packet->battery = get_battery_level();
	memcpy(buffer + sizeof(long) + (sizeof(float) * 5), &packet->battery, sizeof(float));
	Serial.print(", Bat: "); Serial.println(packet->battery);
	packet->chksum = chksum(buffer, 27);
	memcpy(buffer + sizeof(long) + (sizeof(float) * 6), &packet->chksum, sizeof(char));

	lora_send(buffer, 29);

	free(packet);
	return 0;
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
	rf95.setThisAddress(NODE_ADDR);
	rf95.setHeaderFrom(NODE_ADDR);
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

	if (rf95.available()) {
		if (rf95.recv(buf, &len)) {
			Serial.print("Got reply, RSSI: "); Serial.println(rf95.lastRssi(), DEC);
		}
		else {
			Serial.println("Receive failed...");
			return NULL;
		}
	}

	return buf;
}

int process_command(uint8_t* buf) {
	if (memcmp(buf, "fanoff", 6) == 0) {
		Serial.println("Turning Fan OFF");
		fan_on = 0;
		digitalWrite(FAN_PIN, LOW);
	}
	else if (memcmp(buf, "fanon", 6) == 0) {
		Serial.println("Turning Fan ON");
		fan_on = 1;
		digitalWrite(FAN_PIN, HIGH);
	}
	else if (memcmp(buf, "interval", 7) == 0) {
		long interval;
		Serial.print("INTERVAL: ");
		memcpy(&interval, (char*)buf + 8, sizeof(long));
		if (interval >= 0) delayt = interval;
		Serial.println(interval);
	}

	return 0;
}

char chksum(char* buffer, int length) {
	int i = 0;
	char chksum = 1;

	for (i = 0; i < length; i++) {
		chksum ^= buffer[i];
	}
	return chksum;
}