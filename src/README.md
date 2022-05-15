# RFID system
The goal of this project is to develop a simple 3D printer login system for a school 3D printer. The students will get an RFID card to use after they get the required 3D printer skills training. ISIC cards (at least in Czech republic) have an RFID compatible chip built-in, so they can be used instead. 

## Functionality
Every card identifies itself with 4 bytes of data. EEPROM is used to store these values and to load them back in RAM. At the end of the memory, there are several bytes to store indexes of admin cards. Admin cards are allowed to delete users, add users, add other admins or view all the stored cards. Member cards are only allowed to log in.

## Hardware
The project is Arduino-based. It can be used with an Arduino UNO/NANO/MEGA2560 boards. An Adafruit SSD1306 display is being used to display the required information, such as login information, admin card options etc. An MRC522 module is being used to read the card data bytes.

## Required hardware
- Arduino UNO/NANO/MEGA2560
- 5V Relay module
- MRC522 module
- 4 buttons (capacitive)
- A 5V passive buzzer for sound output