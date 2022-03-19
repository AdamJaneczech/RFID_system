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
//Buzzer configuration
#define BUZZER 3
#define TONE_LOW 784
#define TONE_HIGH 1175
//Boolean 8-bit variable configuration
#define ADMIN_CARD 0
#define REGISTERED 1
#define ADMIN_MENU 2
#define PRESSED 3
#define DIM_FLAG 4
#define SCROLL_FLAG 5
#define ALLOWED 6
#define OK_PRESSED 7
//Relay pin
#define RELAY 9
//OLED display name
#define DISPLAY_NAME display
//define neopixel setup constants
#define NEOPIXEL_PIN 5
#define LEDS 24
//Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);
//Initialize NeoPixel LED stripe
//Adafruit_NeoPixel neopixel(LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);	//definice nového LED pásku