#define BOARD_LORASPI

#include <bcm2835.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <RH_RF69.h>
#include <RH_RF95.h>
#include "include/RasPiBoards.h"

#define RF_FREQUENCY  915.00
#define RF_GATEWAY_ID 1
#define RF_NODE_ID    10

#define RF_LED_PIN RPI_V2_GPIO_P1_16 // Led on GPIO23 so P1 connector pin #16
#define RF_LED2_PIN RPI_V2_GPIO_P1_18 // Led on GPIO23 so P1 connector pin #16
#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define RF_RST_PIN RPI_V2_GPIO_P1_15 // IRQ on GPIO22 so P1 connector pin #15

RH_RF95 rf95(RF_CS_PIN, RF_IRQ_PIN);

volatile sig_atomic_t force_exit = false; // Allows ctrl-c exit

void sig_handler(int sig){
  printf("\n%s Break received, exiting!\n", __BASEFILE__);
  force_exit=true;
}

int main (int argc, const char* argv[] ){
  static unsigned long last_millis;
  static unsigned long led_blink = 0;

  signal(SIGINT, sig_handler);
  printf( "%s\n", __BASEFILE__);

  if (!bcm2835_init()) {
    fprintf( stderr, "%s bcm2835_init() Failed\n\n", __BASEFILE__ );
    return 1;
  }

  printf( "RF95 CS=GPIO%d", RF_CS_PIN);

  pinMode(RF_LED_PIN, OUTPUT);
  pinMode(RF_LED2_PIN, OUTPUT);
  digitalWrite(RF_LED_PIN, HIGH );
  digitalWrite(RF_LED2_PIN, HIGH );

  printf( ", IRQ=GPIO%d", RF_IRQ_PIN );
  pinMode(RF_IRQ_PIN, INPUT); // IRQ Pin input/pull down
  bcm2835_gpio_set_pud(RF_IRQ_PIN, BCM2835_GPIO_PUD_DOWN);

  printf( ", RST=GPIO%d", RF_RST_PIN );
  pinMode(RF_RST_PIN, OUTPUT);
  digitalWrite(RF_RST_PIN, LOW );
  bcm2835_delay(150);
  digitalWrite(RF_RST_PIN, HIGH );
  bcm2835_delay(100);

  printf( ", LED=GPIO%d", RF_LED_PIN );
  printf( ", LED2=GPIO%d", RF_LED2_PIN );
  digitalWrite(RF_LED_PIN, LOW );
  digitalWrite(RF_LED2_PIN, LOW );

  if (!rf95.init()) {
    fprintf( stderr, "\nRF95 module init failed, Please verify wiring/module\n" );
  } else {
    printf( "\nRF95 module seen OK!\r\n");

    rf95.available();
    bcm2835_gpio_ren(RF_IRQ_PIN);

    rf95.setTxPower(20, false);
    rf95.setFrequency( RF_FREQUENCY );
    rf95.setThisAddress(RF_NODE_ID);
    rf95.setHeaderFrom(RF_NODE_ID);
    rf95.setHeaderTo(RF_GATEWAY_ID);

    printf("RF95 node #%d init OK @ %3.2fMHz\n", RF_NODE_ID, RF_FREQUENCY );

    last_millis = millis();

    while (!force_exit) {
      // Send every 5 seconds
      if ( millis() - last_millis > 1000 ) {
        last_millis = millis();

        led_blink = millis();
        digitalWrite(RF_LED_PIN, HIGH);

        // Send a message to rf95_server
        uint8_t data[] = "Hi wildfire group!";
        uint8_t len = sizeof(data);

        printf("\nSending %02d bytes to node #%d => ", len, RF_GATEWAY_ID );
        printbuffer(data, len);
        printf("\n" );
        rf95.send(data, len);
        rf95.waitPacketSent();
      }
      bcm2835_delay(100);
    }
  }

  digitalWrite(RF_LED_PIN, LOW );
  printf( "\n%s Ending\n", __BASEFILE__ );
  bcm2835_close();
  return 0;
}
