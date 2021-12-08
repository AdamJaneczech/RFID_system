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
    Serial.println(PORTD, BIN);
    byte bitCheck = PORTD >> 4; //shift PORTD value 4 right -> PD4 at first place, PD7 at 4th place
    bitCheck &= ~(1<<PD5);  //write 0 to the bin place of PD5 (RFID module pin -> not interested)
    Serial.println(bitCheck, BIN);
    if(bitCheck == 0b1){
        Serial.println("OK");
    }
    else if(bitCheck == 0b100){
        Serial.println("UP");    
    }
    else if(bitCheck == 0b1000){
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