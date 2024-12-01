ESP32 in combination with SX1276 module (Hope RF96)
Works with pixmob cement v1.1 (see PCB labeling), for EU (868 MHz). Can be configured to work with 915 MHz using SX1278 module (HopeRF95)
I modified the PixMob waveband library to function with the SX1276 module instead of the TI CC1101. I had the module laying around, planning to use for for LoRa communication
I made a mistake by using the non-default SPI lines of the ESP32. See the hardware connections  below:


|Function | ESP32 | Hope RF96 |
|---------|-------|-----------|
|MOSI | 13 | 3 |   
|MISO | 19 | 2 |
|SCK | 5 | 4 |
|NSS | 18 | 5 |
|DIO0 | 2  | 14 |
|DIO1 | 12 | 15 |
|DIO2 | 17 | 16 |


This repository is based on:
* Example providedvby MajedAbouhatab in https://github.com/MajedAbouhatab/Reusing-PIXMOB-Waveband-Without-Flipper-Zero/tree/main and the accompanying article at hackster.io
* The PixMob Waveband library by sueppchen: https://github.com/sueppchen/PixMob_waveband
* Radiolib: https://github.com/jgromes/RadioLib
