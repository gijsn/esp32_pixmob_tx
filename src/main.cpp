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
  pinMode(RADIO_DIO_2_PORT, OUTPUT);
  SPI.begin(5, 19, 13, 18);
  // SPI.setMOSI(RADIO_MOSI_PORT);
  // SPI.setMISO(RADIO_MISO_PORT);
  // SPI.setSCLK(RADIO_SCLK_PORT);
  radio.beginFSK();
  radio.setFrequency(868.49);
  radio.setOOK(true);
  radio.transmitDirect();
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

/*
void transmitBit(bool txBit) {
  digitalWrite(DIO1, HIGH);
  digitalWrite(DIO2, txBit);
  delayMicroseconds(250);
  digitalWrite(DIO1, LOW);
  delayMicroseconds(250);
}

void transmitByte(byte txByte) {
  for (int bitCounter = 7; bitCounter >= 0; bitCounter--) {
    bool bit = (((txByte) >> (bitCounter)) & 0x01);
    transmitBit(bit);
  }
}

void transmitArray(byte* buffer, byte size) {
  delayMicroseconds(800);  // wait until hf is present
  // preamble
  transmitByte(0xAA);
  transmitByte(0xAA);
  // resync
  transmitBit(0);
  transmitBit(1);
  // payload
  for (byte byteCounter = 0; byteCounter < size; byteCounter++)
    transmitByte(buffer[byteCounter]);
  radio.standby();
}

void setup() {
  pinMode(DIO1, OUTPUT);
  pinMode(DIO2, OUTPUT);
  Serial.begin(115200);

  // initialize SX1278 with default settings
  Serial.print(F("[SX1278] Initializing ... "));
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

  // Set bitrate to 5 kbps
  state = radio.setBitRate(5.0);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("setBitRate failed, code "));
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
  printTableHeader();
  printPrompt();
}
byte ByteArray[17][9] = {
    // 0     1     2     3     4     5     6     7     8
    {0x56, 0x84, 0x84, 0x84, 0x84, 0x62, 0x36, 0x84, 0x29},  // 1 nothing_Black
    {0x94, 0x84, 0x91, 0xb5, 0x84, 0x8c, 0x45, 0x84, 0xad},  // 2 gold_fadein_slow
    {0x6d, 0x84, 0x91, 0xb5, 0x84, 0x49, 0x45, 0x84, 0x8a},  // 3 gold_fastin_fade
    {0x42, 0x84, 0x91, 0xb5, 0x84, 0x64, 0x6a, 0x84, 0x49},  // 4 rand_gold_blink
    {0x4a, 0x84, 0x91, 0xb5, 0x84, 0x89, 0xa9, 0x84, 0x34},  // 5 rand_gold_fade
    {0x56, 0x84, 0x91, 0xb5, 0x84, 0x29, 0x65, 0x84, 0x61},  // 6 rand_gold_fastfade
    {0xa4, 0x84, 0x84, 0xb5, 0x84, 0x89, 0x89, 0x84, 0x65},  // 7 rand_red_fade
    {0x4c, 0x84, 0x84, 0xb5, 0x84, 0x29, 0x65, 0x84, 0x45},  // 8 rand_red_fast_fade
    {0x6d, 0x84, 0x84, 0xb5, 0x84, 0x64, 0x6a, 0x84, 0x29},  // 9 rand_red_fast_blink
    {0xa6, 0x84, 0x84, 0xb5, 0x84, 0x8c, 0x45, 0x84, 0xa1},  // a wine_fade_in
    {0x36, 0x84, 0xb5, 0x84, 0xb1, 0xb6, 0x4d, 0x84, 0x91},  // b rand_turq_blink
    {0x84, 0x84, 0x32, 0x84, 0xb5, 0x89, 0x89, 0x84, 0x36},  // c rand_blue_fade
    {0x4a, 0x84, 0xb5, 0xb5, 0xb5, 0x64, 0x6a, 0x84, 0x89},  // d rand_white_blink
    {0x65, 0x84, 0xb5, 0xb5, 0xb5, 0x89, 0x89, 0x84, 0xad},  // e rand_white_fade
    {0x9a, 0x84, 0xb5, 0xb5, 0xb5, 0x29, 0x65, 0x84, 0xa9},  // f rand_white_fast_fade
    {0x5a, 0x84, 0xb5, 0xb5, 0xb5, 0x49, 0x45, 0x84, 0x2c},  // g white_fast_fade
    {0x5a, 0x84, 0xb5, 0xb5, 0xb5, 0x49, 0x45, 0x84, 0x2c}   // white_fast_fade -- TEST
};

// ------------- CLI ------------------
void clearSerial() {           // Read and discard data on serial port
  while (Serial.available() > 0) {
    Serial.read();
  }
}

byte readCliByte() {           // Read cli byte
  byte input = 0;
  if (Serial.available() > 0) {
    input = Serial.read();
    Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
    Serial.println("");
    clearSerial();
  }
  return input;
}

void printPrompt() {           // Print the prompt
  Serial.print(PROMPT);
}

void printTableHeader() {      // Print result table header
  Serial.println(SEPARATOR);
  Serial.println(TABLE_HEADER);
  Serial.println(SEPARATOR);
}

void commandLineInterface() {  // Minimalistic command line interface
  char selection = readCliByte();
  switch (selection) {
    case '0':
      ind = 0x00;
      break;
    case '1':
      ind = 0x01;
      break;
    case '2':
      ind = 0x02;
      break;
    case '3':
      ind = 0x03;
      break;
    case '4':
      ind = 0x04;
      break;
    case '5':
      ind = 0x05;
      break;
    case '6':
      ind = 0x06;
      break;
    case '7':
      ind = 0x07;
      break;
    case '8':
      ind = 0x08;
      break;
    case '9':
      ind = 0x09;
      break;
    case 'a':
      ind = 0x0a;
      break;
    case 'b':
      ind = 0x0b;
      break;
    case 'c':
      ind = 0x0c;
      break;
    case 'd':
      ind = 0x0d;
      break;
    case 'e':
      ind = 0x0e;
      break;
    case 'f':
      ind = 0x0f;
      break;
    case 'g':
      ind = 0x10;
      break;
    case 'h':
      ind = 0x10;
      break;
    case '?':
      Serial.println(SEPARATOR);
      Serial.println("|                   PixMob                 |");
      Serial.println(SEPARATOR);
      Serial.println(" 0 - transmitter off");
      Serial.println(" 1 - black, wakeup!");
      Serial.println(" 2 - gold, fade in, slow");
      Serial.println(" 3 - gold, fade in, fast");
      Serial.println(" 4 - gold, pulse, random slow");
      Serial.println(" 5 - gold, fade, random slow");
      Serial.println(" 6 - gold, fade, random fast");
      Serial.println(" 7 - red, fade, random slow");
      Serial.println(" 8 - red, fade, random fast");
      Serial.println(" 9 - red, pulse, random fast");
      Serial.println(" a - wine, fade in, slow");
      Serial.println(" b - turq, pulse, random slow");
      Serial.println(" c - blue, fade, random slow");
      Serial.println(" d - white, pulse, random slow");
      Serial.println(" e - white, fade in, random slow");
      Serial.println(" f - white, fade, random fast");
      Serial.println(" g - white, fade in, fast");
      Serial.println(" h - test-signal");
      Serial.println(SEPARATOR);
      break;
    default:
      break;
  }
  if (byte(selection) > 0) {
    printPrompt();
    // Serial.print(byte(selection));
  }
}

// ---------------------- MAIN -------------------
void loop() {
  commandLineInterface();
  if (ind > 0) transmitArray(ByteArray[ind - 1], 9);
  delay(200);
  indexAlt = ind;
}
*/