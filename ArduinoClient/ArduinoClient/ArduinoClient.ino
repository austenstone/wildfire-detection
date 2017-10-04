#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3

#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
	pinMode(RFM95_RST, OUTPUT);
	digitalWrite(RFM95_RST, HIGH);

	Serial.begin(9600);

	Serial.println("Arduino LoRa TX Test!");

	// Reset
	digitalWrite(RFM95_RST, LOW);
	delay(10);
	digitalWrite(RFM95_RST, HIGH);
	delay(10);

	while (!rf95.init()) {
		Serial.println("LoRa radio init failed"); for (;;);
	}
	Serial.println("LoRa radio init OK!");

	if (!rf95.setFrequency(RF95_FREQ)) {
		Serial.println("setFrequency failed"); for (;;);
	}
	Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

	rf95.setTxPower(23, false);	// The default transmitter power is 13dBm, using PA_BOOST but you can set transmitter powers from 5 to 23 dBm:
	rf95.setThisAddress(2);
	rf95.setHeaderFrom(2);
	rf95.setHeaderTo(3);
}

int16_t packetnum = 0;

void loop() {
	char radiopacket[20] = "Hello Wildfire #";
	itoa(packetnum++, radiopacket + 16, 10);
	Serial.print("Sending "); Serial.println(radiopacket);
	radiopacket[19] = 0;

	Serial.print("Sending...");
	rf95.send((uint8_t *)radiopacket, sizeof(radiopacket));

	if (rf95.waitPacketSent(10000) != 0) {
		Serial.println(" Unsucessful Transmission");
	} else {
		Serial.println(" Sent!");
	}

						   // Now wait for a reply
	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(buf);

	delay(10);
	if (rf95.waitAvailableTimeout(1000)) {
		if (rf95.recv(buf, &len)) {
			Serial.print("Got reply: "); Serial.print((char*)buf);
			Serial.print(", RSSI: "); Serial.println(rf95.lastRssi(), DEC);
		} else {
			Serial.println("Receive failed");
		}
	} else {
		Serial.println("No reply, is there a listener around?");
	}
}
