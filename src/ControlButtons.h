#include <avr/io.h>
#include <avr/interrupt.h>

void logout(void);
void homeScreen(void);
void clearDimTimer(void);

ISR(PCINT0_vect){   //STOP_BUTTON
    cli();
    if((global & 1 << ALLOWED) || (global & 1 << ADMIN_MENU)){
        Serial.println("STOP");
        global &= ~(1 << DIM_FLAG);
        clearDimTimer();
        OCR1B = 1563;    //100 ms beep
        TIMSK1 |= (1 << OCIE1B);
        tone(3, TONE_HIGH, 100);
        global &= ~(1 << ALLOWED);
        global &= ~(1 << ADMIN_MENU);
        global &= ~(1 << ADMIN_CARD); //if STOP_BUTTON is pushed during admin option selection, the next condition won't be fulfilles 
        digitalWrite(RELAY, HIGH);
        //delay(100);
    }
    sei();
}

ISR(PCINT2_vect){  //Control buttons
    cli();
    byte bitCheck = (PIND &= ~(1 << PD5)) >> 4; //shift PIND value 4 right -> PD4 at first place, PD7 at 4th place
    //Serial.println(bitCheck, BIN);
    global ^= (1 << PRESSED);
    if((global & 1 << PRESSED)){
        if(bitCheck == 0b1){    //OK button
            if(global & 1 << ADMIN_MENU){
                OCR1B = 250;    //16 ms beep
                Serial.println("OK");
                global &= ~(1 << ADMIN_MENU);
                global &= ~(1 << DIM_FLAG);
                tone(BUZZER, TONE_LOW, 16);
                clearDimTimer();
                TIMSK1 |= (1 << OCIE1B);
            }
            else{
                option = 0;
            }
        }
        else if(bitCheck == 0b100){ //UP button
            //Serial.println("UP");
            if(global & 1 << ADMIN_MENU){
                global &= ~(1 << DIM_FLAG);
                tone(BUZZER, TONE_LOW, 16);
                clearDimTimer();
                OCR1B = 250;    //16 ms beep
                TIMSK1 |= (1 << OCIE1B);
                if(option < maxOption && option >= 0){
                    option++;
                }
                Serial.print("Option ");
                Serial.print(option);
                Serial.print(": ");
            }
        }
        else if(bitCheck == 0b1000){ //DOWN button
            //Serial.println("DOWN");
            if(global & 1 << ADMIN_MENU){
                global &= ~(1 << DIM_FLAG);
                tone(BUZZER, TONE_LOW, 16);
                clearDimTimer();
                OCR1B = 250;    //16 ms beep
                TIMSK1 |= (1 << OCIE1B);
                if(option > 0 && option <= maxOption){
                    option--;
                }
                Serial.print("Option ");
                Serial.print(option);
                Serial.print(": ");
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