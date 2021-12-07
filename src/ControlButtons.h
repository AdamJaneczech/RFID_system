#include <avr/io.h>
#include <avr/interrupt.h>

boolean allowed = false;

void logout(void);

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

ISR(PCINT2_vect){  //Control buttons
    cli();
    //alternative if statement
    Serial.println(PORTB, BIN);
    byte bitCheck = PORTB >> 2;
    Serial.println(bitCheck, BIN);
    bitCheck = bitCheck << 7;
    Serial.println(bitCheck, BIN);
    if(PORTB == PORTB ^ (1<<PB4)){
        Serial.println("OK");
    }
    if(PORTB == PORTB ^ (1<<PB6)){
        Serial.println("UP");    
    }
    if(PORTB == PORTB ^ (1<<PB7)){
        Serial.println("DOWN");
    }
    //option = (option == 4)? option = 4: option++;
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