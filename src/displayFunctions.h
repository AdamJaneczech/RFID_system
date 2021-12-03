#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

#define DISPLAY_NAME display

Adafruit_SSD1306 DISPLAY_NAME(128, 64, &Wire, -1);

void clearDisplayLine(byte line, byte fontSize){
    
}

void printText(char text [], uint8_t x, uint8_t y, uint8_t fontSize, uint8_t fontColor){
    DISPLAY_NAME.setTextSize(fontSize);
    DISPLAY_NAME.setTextColor(fontColor);
    DISPLAY_NAME.setCursor(x, y);
    DISPLAY_NAME.print(text);
}

void printText(int text, uint8_t x, uint8_t y, uint8_t fontSize, uint8_t fontColor){
    DISPLAY_NAME.setTextSize(fontSize);
    DISPLAY_NAME.setTextColor(fontColor);
    DISPLAY_NAME.setCursor(x, y);
    DISPLAY_NAME.print(text);
}

void printText(char text[], uint8_t x, uint8_t y){
    DISPLAY_NAME.setCursor(x, y);
    DISPLAY_NAME.print(text);
}

void printText(int text, uint8_t x, uint8_t y){
    DISPLAY_NAME.setCursor(x, y);
    DISPLAY_NAME.print(text);
}

void displayText(char text [], uint8_t x, uint8_t y, uint8_t fontSize, uint8_t fontColor){
    printText(text, x, y, fontSize, fontColor);
    DISPLAY_NAME.display();
}

void displayText(char text[]){
    DISPLAY_NAME.print(text);
    DISPLAY_NAME.display();
}

void displayText(int text){
    DISPLAY_NAME.print(text);
    DISPLAY_NAME.display();
}