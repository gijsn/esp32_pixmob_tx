/*
  RadioLib Morse Transmit AM Example

  This example sends Morse code message using
  SX1278's FSK modem. The signal is modulated
  as OOK, and may be demodulated in AM mode.

  Other modules that can be used for Morse Code
  with AM modulation:
  - SX127x/RFM9x
  - RF69
  - SX1231
  - CC1101
  - Si443x/RFM2x

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/
#include <Arduino.h>
#include <SPI.h>
// include the library
#include <RadioLib.h>

#include "pixmob_cement.h"

void printTableHeader();
void printPrompt();
// clock out signal DIO!
// data signal DIO2

#define DIO1 12
#define RADIO_DIO_2_PORT 17
#define RADIO_MOSI_PORT 18
#define RADIO_MISO_PORT 19
#define RADIO_SCLK_PORT 5
// ------------ CLI ---------
#define SEPARATOR "+-------------------------------------------+"
#define TABLE_HEADER "| PixMob CLI-interface: type ? for help     |"
#define PROMPT "> "

byte ind, indexAlt;
// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  14
// SX1278 radio = new Module(10, 2, 9, 3);
SX1276 radio = new Module(18, 2, 14, 12);

// or detect the pinout automatically using RadioBoards
// https://github.com/radiolib-org/RadioBoards
/*
#define RADIO_BOARD_AUTO
#include <RadioBoards.h>
Radio radio = new RadioModule();
*/

#include <RadioLib.h>

Pixmob batch;  // create the pixmob instance

void activeDelay(uint16_t time) {
  for (uint16_t i = 0; i < (time / 40); i++) {
    delay(40);
    batch.refresh();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RADIO_DIO_2_PORT, OUTPUT);
  SPI.begin(5, 19, 13, 18);
  // SPI.setMOSI(RADIO_MOSI_PORT);
  // SPI.setMISO(RADIO_MISO_PORT);
  // SPI.setSCLK(RADIO_SCLK_PORT);

  // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
  // Initialize the radio
  int state = radio.beginFSK();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // Set frequency to 433.9 MHz
  state = radio.setFrequency(868.49);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("setFrequency failed, code "));
    Serial.println(state);
  }
  // Set OOK modulation
  state = radio.setOOK(true);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("setOOK failed, code "));
    Serial.println(state);
  }

  state = radio.transmitDirect();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("transmit direct failed, code "));
    Serial.println(state);
  }
  Serial.println("done");

  if (!batch.begin(RADIO_DIO_2_PORT)) {
    Serial.println("CC1101 init fail");
    while (true);                             // loop forever
  } else {
    Serial.println("CC1101 init done");
    delay(1000);
  }
  batch.sendColor(0, 0, 0, 0);
  batch.sendColor(0, 0, 0, 0);
  activeDelay(2000);
  return;
  Serial.println("starting red");
  batch.storeColorSilent(128, 0, 0, 0);       // red
  activeDelay(80);                            //  repeat sending 2 times
  Serial.println("starting orange");
  batch.storeColorSilent(96, 32, 0, 1);       // orange
  activeDelay(80);                            //  repeat sending 2 times
  Serial.println("starting yellow");
  batch.storeColorSilent(64, 64, 0, 2);       // yellow
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(32, 96, 0, 3);       // light green
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 128, 0, 4);       // green
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 96, 32, 5);       // cold green
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 64, 64, 6);       // turkis
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 32, 96, 7);       // aqua
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 0, 128, 8);       // blue
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(32, 0, 96, 9);       // warm blue
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(64, 0, 64, 10);      // violett
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(96, 0, 32, 11);      // purple
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(96, 64, 10, 12);     // warm white
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(40, 40, 40, 13);     // cold white
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(0, 0, 0, 14);        // black
  activeDelay(80);                            //  repeat sending 2 times
  batch.storeColorSilent(255, 255, 255, 15);  // full on
  activeDelay(80);                            //  repeat sending 2 times

  batch.setFXtiming(7, 6, 0, 0);              // slowest attack, longest hold, be background for fading, no random
  batch.playMemForever(0, 11, 0);             // fade color 0 to color 11
  activeDelay(80);                            //  repeat sending 2 times
}

#define RED 128                               // 0 - 255
#define GREEN 30                              // 0 - 255
#define BLUE 0                                // 0 - 255
#define ATTACK 1                              // 0 -  7 (0 = fast)
#define HOLD 2                                // 0 -  7 (7 = forever)
#define RELEASE 2                             // 0 -  7 (0 = background color)
#define RANDOM 2                              // 0 -  7 (0 = no random)
#define GROUP 0                               // 0 - 31 (0 = all batches)

void loop() {
  return;
  batch.setFXtiming(0, 0, 0, 0);
  batch.sendColor(0, 0, 0, 0);
  // activeDelay(5000);
  batch.setFXtiming(ATTACK, HOLD, RELEASE, RANDOM);
  batch.sendColor(RED, GREEN, BLUE, GROUP);
  activeDelay(5000);
  batch.flashDual(128, 0, 0, 0, 0, 128);
  activeDelay(2000);
}
