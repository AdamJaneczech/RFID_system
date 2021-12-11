#include <avr/io.h>
#include <avr/interrupt.h>
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
#define UP_BUTTON PD6
#define DOWN_BUTTON PD7
#define OK_BUTTON PD4
#define STOP_BUTTON PB0
//Relay pin
#define RELAY 9
//OLED display name
#define DISPLAY_NAME display
//Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);