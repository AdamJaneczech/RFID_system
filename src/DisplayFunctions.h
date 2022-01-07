#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

Adafruit_SSD1306 DISPLAY_NAME(128, 64, &Wire, -1);

//Display dim timer setup
void displayDimSetup(){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0; //counter value = 0
    TCCR1B |= (1 << WGM12)|(1 << CS12)|(1 << CS10);    //set the waveform generation mode & prescaler to clk/1024 (datasheet)
    OCR1A |= 62500; //4-second interval
    OCR1B |= 32500; //4-second interval
    TIMSK1 |= (1 << OCIE1B)|(1 << OCIE1A);   //output compare match enable
}

ISR(TIMER1_COMPA_vect){
    cli();  //disable interrupts
    global |= 1 << DIM_FLAG;
    sei();  //enable interrupts
}

ISR(TIMER1_COMPB_vect){
    cli();  //disable interrupts
    global |= 1 << SCROLL_FLAG; //Write 1 to the scroll flag
    sei();  //enable interrupts
}

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

void printText(const char text[], uint8_t x, uint8_t y, uint8_t fontSize, uint8_t fontColor){
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
    clearDimTimer();
    DISPLAY_NAME.dim(false);

    DISPLAY_NAME.clearDisplay();
    DISPLAY_NAME.drawBitmap(128-56,0,gymkrenLogo,56,56,BLACK, WHITE);

    printText((char*)"Login", 2, 3, 2, WHITE);
    DISPLAY_NAME.drawRoundRect(0,0,64,21,2,WHITE);
    DISPLAY_NAME.fillRect(0,56,128,8,BLACK);

    clearDisplayLine(7, 1);
    printText((char*)"Last user: ", 0, 56, 1, WHITE);
    displayText(actualCardIndex);
    DISPLAY_NAME.display();
}

void homeScreen(boolean firstTimeDisplay){
    DISPLAY_NAME.clearDisplay();
    DISPLAY_NAME.drawBitmap(128-56,0,gymkrenLogo,56,56,BLACK, WHITE);

    printText((char*)"Login", 2, 3, 2, WHITE);
    DISPLAY_NAME.drawRoundRect(0,0,64,21,2,WHITE);
    DISPLAY_NAME.fillRect(0,56,128,8,BLACK);

    if(firstTimeDisplay){
        printText((char*)"Saved cards: ", 0, 56, 1, WHITE);
        displayText(cardCount);
    }
}