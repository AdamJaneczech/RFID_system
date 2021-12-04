#include <Arduino.h>

//Serial communication baud rate
#define BAUDRATE 115200
//MFRC522 SPI pins
#define SS_PIN 10
#define RST_PIN 9
//Card configuration
#define ADMIN_CARDS 8
#define MAX_EEPROM 1023
#define MAX_CARDS (MAX_EEPROM+1)/4 - ADMIN_CARDS
//Button configuration
#define UP_BUTTON PCINT22
#define DOWN_BUTTON PCINT23
#define OK_BUTTON PCINT20
#define STOP_BUTTON PCINT0
//Relay pin
#define RELAY 9
//OLED display name
#define DISPLAY_NAME display