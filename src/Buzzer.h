#include <Arduino.h>

void buzzerSetup(){ //Setup the frequencies, timer registers, etc. for the buzzer
    pinMode(3, OUTPUT);
    TCCR2A |= (1 << WGM21)|(1 << WGM20);  // Fast PWM mode, OCR1A = TOP
    TCCR2B |= (1 << WGM22)|(1 << CS22)|(1 << CS20); //Fast PWM mode, OCR1A = TOP; set prescaler to clk / 128
    TCCR2A |= (1 << COM2B1);  //Clear OC2B on compare match, set OC2B at BOTTOM (non-inverting mode);
    OCR2A = 159;
    OCR2B = 159;    //Change!!
}

/*ISR(TIMER2_COMPA_vect){ //When OCR2A counts to the max value, increment/decrement the value
    
}*/