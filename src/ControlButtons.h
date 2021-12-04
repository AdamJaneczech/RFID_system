#include <avr/io.h>
#include <avr/interrupt.h>

boolean allowed = false;

ISR(PCINT0_vect){
    cli();
    allowed = false;
    digitalWrite(RELAY, HIGH);
    delay(100);
    sei();
}

ISR(PCINT20_vect){
    cli();
    //
    sei();
}

ISR(PCINT22_vect){
    cli();
    //
    sei();
}

ISR(PCINT23_vect){
    cli();
    //
    sei();
}



void interruptConfig(void){
    SREG |= (1<<7); //Global interrupt enable
    PCICR |= _BV(PCIE0);
    PCMSK0 |= _BV(PCINT0);
}