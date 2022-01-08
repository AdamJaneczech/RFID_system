#include <Arduino.h>

void buzzerSetup(){ //Setup the frequencies, timer registers, etc. for the buzzer
    pinMode(3, OUTPUT);
    TCCR2A = 0; //clear the registers to make a custom timer setup
    TCCR2B = 0;
    TCCR2A |= (1 << WGM21)|(1 << WGM20);  // Fast PWM mode, OCR1A = TOP
    TCCR2B |= (1 << WGM22)|(1 << CS20); //Fast PWM mode, OCR1A = TOP; no prescaler
    TCCR2A |= (1 << COM2B1);  //Clear OC2B on compare match, set OC2B at BOTTOM (non-inverting mode);
    TIMSK2 |= (1 << OCIE2A);  //Set the compare match A interrupt enabled  
    OCR2A = 101;
    OCR2B = 20;    //Change!!
}

ISR(TIMER2_COMPA_vect){ //When OCR2A counts to the max value, increment/decrement the value
    cli();  //disable interrupts
    if(OCR2B == 101 || OCR2B == 0){
        global ^= (1 << BUZZER_WAVE);
        //Serial.println(OCR2B);
    }
    if(global & (1 << BUZZER_WAVE)){
        OCR2B += 1;
    }
    else{
        OCR2B -= 1;
    }
    sei();  //enable interrupts
}