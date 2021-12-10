#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

Adafruit_SSD1306 DISPLAY_NAME(128, 64, &Wire, -1);

void clearDisplayLine(byte line, byte fontSize){
    DISPLAY_NAME.fillRect(0,line * 8,128,fontSize * 8,BLACK);
    if(line < 7){
        DISPLAY_NAME.drawBitmap(128-56,0,gymkrenLogo,56,56,BLACK, WHITE);
    }
    DISPLAY_NAME.display();
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

void homeScreen(){
    DISPLAY_NAME.clearDisplay();
    DISPLAY_NAME.drawBitmap(128-56,0,gymkrenLogo,56,56,BLACK, WHITE);

    printText("Login", 2, 2, 2, WHITE);
    DISPLAY_NAME.drawRoundRect(0,0,64,20,2,WHITE);
    DISPLAY_NAME.fillRect(0,56,128,8,BLACK);

    clearDisplayLine(7, 1);
    printText("Last user: ", 0, 56, 1, WHITE);
    displayText(actualCardIndex);
    DISPLAY_NAME.display();
}

void homeScreen(boolean firstTime){
    DISPLAY_NAME.clearDisplay();
    DISPLAY_NAME.drawBitmap(128-56,0,gymkrenLogo,56,56,BLACK, WHITE);

    printText("Login", 2, 2, 2, WHITE);
    DISPLAY_NAME.drawRoundRect(0,0,64,20,2,WHITE);
    DISPLAY_NAME.fillRect(0,56,128,8,BLACK);

    printText("Saved cards: ", 0, 56, 1, WHITE);
    displayText(cardCount);
}