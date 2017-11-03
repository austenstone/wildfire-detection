// hello world
#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <RH_RF69.h>
#include <RH_RF95.h>
#include <RH_TCP.h>

#define BOARD_LORASPI
#include "include/RasPiBoards.h"

struct WildfirePacket {
	int loop_count;
	int co2;
	int co2_temp;
	int ozone_gas;
	int temp;
	int ppm;
};

#define RF_RED_LED_PIN RPI_V2_GPIO_P1_16 // Led on GPIO24 so P1 connector pin #18
#define RF_YELLOW_LED_PIN RPI_V2_GPIO_P1_18 // Led on GPIO24 so P1 connector pin #18
#define RF_GREEN_LED_PIN RPI_V2_GPIO_P1_11 // Led on GPIO24 so P1 connector pin #11
#define RF_BLUE_LED_PIN RPI_V2_GPIO_P1_12 // Led on GPIO24 so P1 connector pin #11
#define RF_FREQUENCY  915.00
#define RF_GATEWAY_ID 1337
#define RF_NODE_ID    1

RH_RF95 rf95(RF_CS_PIN, RF_IRQ_PIN);

void lightshow(int interations);
void warning(int interations);

volatile sig_atomic_t force_exit = false;

void sig_handler(int sig) {
	printf("\n%s Break received, exiting!\n", __BASEFILE__);
	force_exit = true;
}

int main(int argc, const char* argv[]) {
	signal(SIGINT, sig_handler);
	printf("%s\n", __BASEFILE__);

	if (!bcm2835_init()) {
		fprintf(stderr, "%s bcm2835_init() Failed\n\n", __BASEFILE__);
		return 1;
	}

	printf("RF95 CS=GPIO%d", RF_CS_PIN);

	printf(", LED(RBGY)=GPIO%d", RF_RED_LED_PIN, RF_YELLOW_LED_PIN, RF_GREEN_LED_PIN, RF_BLUE_LED_PIN);
	printf(", LED2=GPIO%d", RF_YELLOW_LED_PIN);
	pinMode(RF_RED_LED_PIN, OUTPUT);
	pinMode(RF_YELLOW_LED_PIN, OUTPUT);
	pinMode(RF_GREEN_LED_PIN, OUTPUT);
	pinMode(RF_BLUE_LED_PIN, OUTPUT);
	lightshow(1);
	printf("Step 1");

	printf(", IRQ=GPIO%d", RF_IRQ_PIN);
	pinMode(RF_IRQ_PIN, INPUT); // IRQ Pin input/pull down
	bcm2835_gpio_set_pud(RF_IRQ_PIN, BCM2835_GPIO_PUD_DOWN); // Now we can enable Rising edge detection
	bcm2835_gpio_ren(RF_IRQ_PIN);

	printf(", RST=GPIO%d", RF_RST_PIN);
	pinMode(RF_RST_PIN, OUTPUT);
	digitalWrite(RF_RST_PIN, LOW);
	bcm2835_delay(150);
	digitalWrite(RF_RST_PIN, HIGH);
	bcm2835_delay(100);

	if (!rf95.init()) {
		fprintf(stderr, "\nRF95 module init failed, Please verify wiring/module\n");
		while (true) {
		}
	} else {
		rf95.setTxPower(23, false);
		rf95.setFrequency(RF_FREQUENCY);
		rf95.setThisAddress(RF_NODE_ID);
		rf95.setHeaderFrom(RF_NODE_ID);
		rf95.setPromiscuous(true);
		rf95.setModeRx();

		printf(" OK NodeID=%d @ %3.2fMHz\n", RF_NODE_ID, RF_FREQUENCY);
		printf("Listening packet...\n");

		while (!force_exit) {
			digitalWrite(RF_YELLOW_LED_PIN, LOW);
			if (bcm2835_gpio_eds(RF_IRQ_PIN)) {
				digitalWrite(RF_YELLOW_LED_PIN, HIGH);
				bcm2835_gpio_set_eds(RF_IRQ_PIN);
				digitalWrite(RF_YELLOW_LED_PIN, LOW);
				if (rf95.available()) {
					digitalWrite(RF_RED_LED_PIN, LOW);
					uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
					uint8_t len = sizeof(buf);
					uint8_t from = rf95.headerFrom();
					uint8_t to = rf95.headerTo();
					uint8_t id = rf95.headerId();
					uint8_t flags = rf95.headerFlags();
					int8_t  rssi = rf95.lastRssi();
					if (rf95.recv(buf, &len)) {
						// Received packet
						digitalWrite(RF_YELLOW_LED_PIN, LOW);
						digitalWrite(RF_GREEN_LED_PIN, HIGH);
						//struct WildfirePacket *packet = (WildfirePacket*)malloc(sizeof(WildfirePacket));
						printf("Packet[%02d] #%d => #%d %ddB: '%s'\n", len, from, to, rssi, buf);

						// Send reply
						digitalWrite(RF_BLUE_LED_PIN, HIGH);
						rf95.setModeTx();
						uint8_t data[] = "Did you send me this?";
						uint8_t len = sizeof(data);
						printf("[%02d bytes] #%d => %s\n", len, RF_GATEWAY_ID, (char*)data);
						rf95.send(data, len);
						rf95.waitPacketSent();
						digitalWrite(RF_GREEN_LED_PIN, LOW);
						digitalWrite(RF_BLUE_LED_PIN, LOW);
					}
					else {
						digitalWrite(RF_RED_LED_PIN, HIGH);
						bcm2835_delay(500);
						digitalWrite(RF_RED_LED_PIN, LOW);
						Serial.print("Receive failed\n");
					}
				}
				else {
					digitalWrite(RF_RED_LED_PIN, HIGH);
					Serial.print("Receive no aval\n");
				}
			}
			//bcm2835_delay(5); // this will charge CPU usage, take care and monitor
		}
	}
	printf("\n%s Ending\n", __BASEFILE__);
	bcm2835_close();
	return 0;
}

void lightshow(int interations) {
	int x = 5;
	int i = 0;

	for (i = 0; i < interations; i++) {
		while (x < 35) {
			digitalWrite(RF_RED_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_RED_LED_PIN, LOW);
			digitalWrite(RF_YELLOW_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_YELLOW_LED_PIN, LOW);
			digitalWrite(RF_BLUE_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_BLUE_LED_PIN, LOW);
			digitalWrite(RF_GREEN_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_GREEN_LED_PIN, LOW);
			digitalWrite(RF_BLUE_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_BLUE_LED_PIN, LOW);
			digitalWrite(RF_YELLOW_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_YELLOW_LED_PIN, LOW);
			bcm2835_delay(500 / x);
			++x;
		}
		x = 5;
	}
}

void warning(int interations) {
	int x = 5;
	int i = 0;

	for (i = 0; i < interations; i++) {
		while (x < 10) {
			digitalWrite(RF_RED_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_RED_LED_PIN, LOW);
			digitalWrite(RF_YELLOW_LED_PIN, HIGH);
			bcm2835_delay(500 / x);
			digitalWrite(RF_YELLOW_LED_PIN, LOW);
			++x;
		}
		x = 5;
	}
	digitalWrite(RF_RED_LED_PIN, LOW);
	digitalWrite(RF_YELLOW_LED_PIN, LOW);
}

int send(void* data) {
	return 1;
}