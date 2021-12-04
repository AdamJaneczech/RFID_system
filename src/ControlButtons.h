#include <avr/io.h>
#include <avr/interrupt.h>
#include <Config.h>
//#include "RFID_system.cpp"

boolean allowed = false;

ISR(PCINT0_vect){
    cli();
    allowed = false;
    digitalWrite(RELAY, HIGH);
    delay(100);
    sei();
}

void interruptConfig(void){
    SREG |= (1<<7); //Global interrupt enable
    PCICR |= _BV(PCIE0);
    PCMSK0 |= _BV(PCINT0);
}