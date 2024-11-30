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

void printTableHeader();
void printPrompt();
// clock out signal DIO!
// data signal DIO2

#define DIO1 14
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
// DIO1 pin:  3
// SX1278 radio = new Module(10, 2, 9, 3);
SX1276 radio = new Module(18, 2, 14, 14);

// or detect the pinout automatically using RadioBoards
// https://github.com/radiolib-org/RadioBoards
/*
#define RADIO_BOARD_AUTO
#include <RadioBoards.h>
Radio radio = new RadioModule();
*/

#include <RadioLib.h>

unsigned long Timestamp;
// SX1276 radio = new Module(RADIO_NSS_PORT, RADIO_DIO_0_PORT, RADIO_RESET_PORT, RADIO_DIO_1_PORT);
int ColorIndex = 0, BitDuration = 500;
#define ValidValuesCount 4
#define BytesCount 12

std::array<byte, BytesCount> ByteArray;
std::array<std::array<int, BytesCount>, ValidValuesCount> ColorArrayArray{{
    {0xaa, 0xaa, 0x65, 0x21, 0x24, 0x6d, 0x61, 0x23, 0x11, 0x61, 0x2b, 0x40},  // gold_fade_in
    {0xaa, 0xaa, 0x5b, 0x61, 0x24, 0x6d, 0x61, 0x12, 0x51, 0x61, 0x22, 0x80},  // gold_fast_fade
    // {0xaa, 0xaa, 0x94, 0x84, 0x91, 0xb5, 0x84, 0x8c, 0x45, 0x84, 0xa, 0x40d},  // 2 gold_fadein_slow
    // {0xaa, 0xaa, 0x6d, 0x84, 0x91, 0xb5, 0x84, 0x49, 0x45, 0x84, 0x8a, 0x40},  // 3 gold_fastin_fade
    // {0xaa,0xaa,0x55,0xa1,0x21,0x21,0x21,0x18,0x8d,0xa1,0xa,0x40}, //nothing
    // {0xaa, 0xaa, 0x61, 0x21, 0xc, 0xa1, 0x2d, 0x62, 0x62, 0x61, 0xd, 0x80}, // rand_blue_fade
    // {0xaa,0xaa,0x50,0xa1,0x24,0x6d,0x61,0x19,0x1a,0xa1,0x12,0x40}, //rand_gold_blink
    // {0xaa,0xaa,0x52,0xa1,0x24,0x6d,0x61,0x22,0x6a,0x61,0xd}, //rand_gold_fade
    // {0xaa,0xaa,0x55,0xa1,0x24,0x6d,0x61,0xa,0x59,0x61,0x18,0x40}, //rand_gold_fastfade
    // {0xaa,0xaa,0x69,0x21,0x21,0x2d,0x61,0x22,0x62,0x61,0x19,0x40}, //rand_red_fade
    // {0xaa,0xaa,0x5b,0x61,0x21,0x2d,0x61,0x19,0x1a,0xa1,0xa,0x40}, //rand_red_fastblink
    // {0xaa,0xaa,0x53,0x21,0x21,0x2d,0x61,0xa,0x59,0x61,0x11,0x40}, //rand_red_fastfade
    // {0xaa,0xaa,0x4d,0xa1,0x2d,0x61,0x2c,0x6d,0x93,0x61,0x24,0x40}, //rand_turq_blink
    // {0xaa,0xaa,0x52,0xa1,0x2d,0x6d,0x6d,0x59,0x1a,0xa1,0x22,0x40}, //rand_white_blink
    // {0xaa,0xaa,0x59,0x61,0x2d,0x6d,0x6d,0x62,0x62,0x61,0x2b,0x40}, //rand_white_fade
    // {0xaa,0xaa,0x66,0xa1,0x2d,0x6d,0x6d,0x4a,0x59,0x61,0x2a,0x40}, //rand_white_fastfade
    {0xaa, 0xaa, 0x56, 0xa1, 0x2d, 0x6d, 0x6d, 0x52, 0x51, 0x61, 0xb},         // white_fastfade
    {0xaa, 0xaa, 0x69, 0xa1, 0x21, 0x2d, 0x61, 0x23, 0x11, 0x61, 0x28, 0x40},  // wine_fade_in
}};

void ByteArraySend(void) {
  for (int j = 0; j < sizeof(ByteArray); j++)
    for (int i = 0; i < 8; i++) {
      digitalWrite(RADIO_DIO_2_PORT, (128U & ByteArray[j]) / 128U);
      delayMicroseconds(BitDuration);
      ByteArray[j] <<= 1;
    }
  digitalWrite(RADIO_DIO_2_PORT, 0);
  for (int k = 0; k < 8; k++)
    delayMicroseconds(BitDuration);
}

void setup() {
  Serial.begin(115200);
  pinMode(RADIO_DIO_2_PORT, OUTPUT);
  SPI.begin(5, 19, 13, 18);
  // SPI.setMOSI(RADIO_MOSI_PORT);
  // SPI.setMISO(RADIO_MISO_PORT);
  // SPI.setSCLK(RADIO_SCLK_PORT);
  radio.setOOK(true);
  radio.transmitDirect();

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
}

void loop() {
  ByteArray = {0xaa, 0xaa, 0x55, 0xa1, 0x21, 0x21, 0x21, 0x18, 0x8d, 0xa1, 0x0a, 0x40};
  ByteArraySend();
  if (millis() - Timestamp > 2000) {
    Timestamp = millis();
    for (int i = 0; i < BytesCount; i++)
      ByteArray[i] = ColorArrayArray[ColorIndex][i];
    ByteArraySend();
    ColorIndex = ColorIndex++ == ValidValuesCount - 1 ? 0 : ColorIndex;
  }
}
