#include <avr/io.h>
#include <avr/interrupt.h>

boolean allowed = false;

void logout(void);
void homeScreen(void);
void clearDimTimer(void);

ISR(PCINT0_vect){   //STOP_BUTTON
    cli();
    Serial.println("STOP");
    if(allowed || adminMenu){
        dimFlag = false;
        clearDimTimer();
        allowed = false;
        adminMenu = false;
        adminCard = false; //if STOP_BUTTON is pushed during admin option selection, the next condition won't be fulfilles 
        digitalWrite(RELAY, HIGH);
        delay(100);
    }
    sei();
}

ISR(PCINT2_vect){  //Control buttons
    cli();
    //alternative if statement
    Serial.println(PIND, BIN);
    byte bitCheck = (PIND &= ~(1 << PD5)) >> 4; //shift PIND value 4 right -> PD4 at first place, PD7 at 4th place
    Serial.println(bitCheck, BIN);
    pressed ^= pressed;
    if(!pressed){    
        if(bitCheck == 0b1){    //OK button
            //Serial.println("OK");
            if(adminMenu){
                adminMenu = false;
                dimFlag = false;
                clearDimTimer();
            }
            else{
                option = 0;
            }
        }
        else if(bitCheck == 0b100){ //UP button
            //Serial.println("UP");
            if(adminMenu){
                dimFlag = false;
                clearDimTimer();
                if(option < 4){
                    option++;
                }
            }
        }
        else if(bitCheck == 0b1000){ //DOWN button
            //Serial.println("DOWN");
            if(adminMenu){
                dimFlag = false;
                clearDimTimer();
                if(option > 0){
                    option--;
                }
            }
        }
    }
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

void clearDimTimer(){
    TCNT1  = 0; //counter value = 0
}