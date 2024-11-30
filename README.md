ESP32 in combination with SX1276 module (Hope RF96)
Works with pixmob cement v1.1 (see PCB labeling), for EU (868 MHz). Can be configured to work with 915 MHz using SX1278 module (HopeRF95)

I made a mistake by using the non-default SPI lines. See the pinout below:

Function    ESP32        Hope RF96
MOSI        13           3    
MISO        19           2
SCK         5            4
NSS         18           5

DIO0        2            14
DIO1        12           15
DIO2        17           16
