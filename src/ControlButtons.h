#include <avr/io.h>
#include <avr/interrupt.h>

boolean allowed = false;

ISR(PCINT0_vect){   //STOP_BUTTON
    cli();
    if(allowed || adminMenu){
        allowed = false;
        adminMenu = false;
        adminCard = false; //if STOP_BUTTON is pushed during admin option selection, the next condition won't be fulfilles 
        digitalWrite(RELAY, HIGH);
        logout();
        delay(100);
    }
    sei();
}

ISR(PCINT20_vect){  //OK_BUTTON
    cli();
    if(adminMenu){
        adminMenu = false;
    }
    sei();
}

ISR(PCINT22_vect){  //UP_BUTTON
    cli();
    //alternative if statement
    option = (option == 4)? option = 4: option++;
    sei();
}

ISR(PCINT23_vect){  //DOWN_BUTTON
    cli();
    //alternative if statement
    option = (option == 0)? option = 0: option++;
    sei();
}

void interruptConfig(void){
    SREG |= (1<<7); //Global interrupt enable
    PCICR |= _BV(PCIE0);    //Enable interrupts for PCINT0 - 7
    PCICR |= _BV(PCIE2);    //Enable interrupts for PCINT16 - 23
    //Enable pin change interrupts for specific pins
    PCMSK0 |= _BV(PCINT0);
    PCMSK2 |= _BV(PCINT20);
    PCMSK2 |= _BV(PCINT22);
    PCMSK2 |= _BV(PCINT23);
}