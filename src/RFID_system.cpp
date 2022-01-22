#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

byte actualCard[4]; //variable for storing the active card number
byte actualCardIndex; //variable for storing the actual card index
byte cardCount = 1; //variable for storing the amount of loaded cards
byte lowestEmptyIndex;  //the lowest index for storing a card into EEPROM is saved in this variable
byte option = 0;  //selected option - aadmin menu
byte maxOption;
byte selectedCardIndex; //when selecting a specific card for deletion/admin rights/other

volatile uint8_t global = 0b00000000;  //the global booleans are on specific bits (see Config.h)

//admin options to display; their length is calculated; | used to separate the options
const char* adminOptions[] = {(char*)"Login|", (char*)"Add ID|", (char*)"Add admin|", (char*)"Delete ID|", (char*)"View IDs|"};

#include <Config.h>
#include <ControlButtons.h>
#include <OLED_icons.h>
#include <DisplayFunctions.h>

void eepromPrint(){
  //print the entire EEPROM via serial
  Serial.println(F("Memory map:"));
  for(int i = 0; i <= MAX_EEPROM; i++){
    Serial.print(EEPROM.read(i));
    Serial.print(' ');
    if(i % 31 == 0 && i != 0){  //every 32nd element creates a new line
      Serial.println();
    }
  }
}

void cleanSerial(){
  Serial.end();
  Serial.begin(BAUDRATE);
}

void logout(){
  digitalWrite(RELAY, HIGH);  //turn the relay pin HIGH -> switch it off
  tone(BUZZER, TONE_HIGH, 100);
  homeScreen();
  Serial.println(F("Logout"));
}

void login(){
  global &= ~(1 << DIM_FLAG);
  clearDimTimer();
  TIMSK1 |= (1 << OCIE1B);
  tone(BUZZER, TONE_LOW, 100);
  digitalWrite(RELAY, LOW);
  Serial.println(F("Passed. Login OK"));
  global |= 1 << ALLOWED; //interrupt makes this false
  cleanSerial();
  DISPLAY_NAME.fillRect(0,0,128-56,42,BLACK);
  DISPLAY_NAME.fillRect(100,56,128,8,BLACK);  //clear the mysterious sign appearing on the display
  DISPLAY_NAME.fillRoundRect(0,0,72,20,2,WHITE);
  displayText((char*)"Active", 2, 3, 2, BLACK);
  while(!(Serial.available() > 0) && (global & 1 << ALLOWED) && !(global & 1 << DIM_FLAG)){
    //Serial input or PCINT breaks the loop
  }
  DISPLAY_NAME.dim(true);
  while(!(Serial.available() > 0) && (global & 1 << ALLOWED)){
    TIMSK1 &= ~(1 << OCIE1B);
  }
  logout();
}

void getCardInfo(){
  cardCount = 0;
  int loopCount = 0;
  byte emptyFields = 0;
  byte emptyIndex = 0;
  Serial.println(F("Empty indexes:"));
  DISPLAY_NAME.setCursor(0,56);
  while(loopCount <= MAX_EEPROM + 1 - ADMIN_CARDS * 4){ //EEPROM address -> card value has 4 8-bit numbers
    if(EEPROM.read(loopCount) == 0xFF){
      emptyFields++;
    }
    else if((loopCount + 1) % 4 == 0 && emptyFields < 4){ //a 4 loop checking variable/something needs to be added
      cardCount++;
      if(emptyIndex != 0){
        emptyFields = 0;
        //emptyFields = EEPROM.read(loopCount); //temporarily use the emptyFields variable as a cache
      }
    }
    if(emptyFields == 4){
      if(emptyIndex == 0){
        lowestEmptyIndex = ((loopCount + 1) / 4) - 1;
        Serial.print(F("Lowest: "));
        Serial.println(lowestEmptyIndex);
      }
      emptyIndex = ((loopCount + 1) / 4) - 1;
      Serial.print(emptyIndex);
      Serial.print(' ');
      //Serial.println(", ");
      emptyFields = 0;    
    }
    if(loopCount % 8 == 0){
        DISPLAY_NAME.drawPixel(loopCount/8, 63, WHITE);
        DISPLAY_NAME.display();
        if(loopCount % 32 == 0){
          Serial.println();
        }
    }
    loopCount++;
  }
  Serial.println();
}

void getCardNumber(){
  clearDisplayLine(7,1);
  DISPLAY_NAME.fillRect(0,48,128-56,8,BLACK);
  DISPLAY_NAME.fillRect(0,56,128,8,BLACK);
  DISPLAY_NAME.setCursor(0,48);
  for(int i = 0; i < mfrc522.uid.size; i++){
    actualCard[i] = mfrc522.uid.uidByte[i];
    if(actualCard[i] < 0xF){
      Serial.print('0');
      DISPLAY_NAME.print('0');
    }
    Serial.print(actualCard[i], HEX /*+ " "*/);
    DISPLAY_NAME.print(actualCard[i], HEX);
  }
  Serial.println();
  DISPLAY_NAME.display();
}

void isCardRegistered(){ //modified here -> EEPROM direct reading
  Serial.println();
  global &= ~(1 << REGISTERED);
  global &= ~(1 << ADMIN_CARD);
  for(int i = 0; i <= MAX_EEPROM + 1 - ADMIN_CARDS * 4 && !(global & 1 << REGISTERED); i += 4){
    for(int y = i; y < i + 4; y++){
      if(EEPROM.read(y) == actualCard[y-i] && y == i){
        global |= 1 << REGISTERED;
        Serial.println(F("OK"));
      }
      if(EEPROM.read(y) == actualCard[y-i] && y != i && (global & 1 << REGISTERED)){
        Serial.println(F("OK"));
      }
      if(EEPROM.read(y) != actualCard[y-i]){
        global &= ~(1 << REGISTERED);
        break;
      }
    }
    if(global & 1 << REGISTERED){
      actualCardIndex = i/4;  //the card index is 4 bytes long
      DISPLAY_NAME.fillRect(0,56,128,8,BLACK);
      DISPLAY_NAME.setCursor(0,56);
      DISPLAY_NAME.print(F("Registered at: " ));
      DISPLAY_NAME.print(actualCardIndex);
      DISPLAY_NAME.display();
      //
      Serial.print(F("Card registered at: "));
      Serial.println(actualCardIndex);
      for(int index = MAX_EEPROM - ADMIN_CARDS + 1; index <= MAX_EEPROM; index++){
        global |= ((EEPROM.read(index) == i/4) << ADMIN_CARD);
        if(global & 1 << ADMIN_CARD){
          break;
        }
      }
    }
    if(global & 1 << REGISTERED && !(global & 1 << ADMIN_CARD)){
      login();
      break;
    }
    if(!(global & 1 << REGISTERED) && i == MAX_EEPROM + 1 - ADMIN_CARDS * 4){
      Serial.println(F("ID not yet registered"));
      DISPLAY_NAME.setCursor(0,56);
      DISPLAY_NAME.print(F("ID not yet registered"));
    }
  }
  DISPLAY_NAME.display();
  //registered = false;
}

void addCard(){
  Serial.println(F("Approximate new card to the reader..."));
  homeScreen(false);  //don't display yet
  clearDisplayLine(6,2);
  displayText((char*)"Approach the new card", 0, 56, 1, WHITE);
  // Look for new cards
  while( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    ;
  }
  // Select one of the cards
  while( ! mfrc522.PICC_ReadCardSerial()) 
  {
    ;
  }
  mfrc522.PICC_HaltA(); //stop the reading
  //Check if the card is registered
  getCardNumber();
  getCardInfo();
  isCardRegistered();
  //If the card is NOT registered
  if(!(global & 1 << REGISTERED)){
    Serial.println(F("New card number: "));
    for(int i = 0; i < mfrc522.uid.size; i++){
      if(cardCount < ((MAX_EEPROM + 1) / 4) - ADMIN_CARDS){
        EEPROM.write((lowestEmptyIndex) * 4 + i, mfrc522.uid.uidByte[i]); //5th position in EEPROM determines the admin attribute
      }
      actualCard[i] = mfrc522.uid.uidByte[i];
      if(actualCard[i] < 0xF){
        Serial.print('0');
        printText((char*)"0", 0, 48);
      }
      Serial.print(actualCard[i], HEX);
      printText((actualCard[i] && HEX), 0, 48, 1, WHITE);
      //----//
    }
    Serial.println();
    Serial.print(F("Card "));
    for(int i = 0; i < 4; i++){
      Serial.print(actualCard[i], HEX);
      Serial.print(' ');
    }
    Serial.print(F("stored as number "));
    Serial.print(lowestEmptyIndex);
    Serial.println(F(" in the database."));
    cardCount++;
    Serial.print(F("Actual number of stored cards: ")); 
    Serial.println(cardCount);
    clearDisplayLine(7,1);
    printText((char*)"New card index: ", 0, 56, 1, WHITE);
    displayText(lowestEmptyIndex);  //display all other text & shapes defined before
    clearDimTimer();
    global &= ~(1 << DIM_FLAG);
    while(global & 1 << ADMIN_MENU && !(global & 1 << DIM_FLAG) && !(Serial.available() > 0)){
      ;
    }
    DISPLAY_NAME.dim(true);
    while(global & 1 << ADMIN_MENU && !(Serial.available() > 0)){
      ;
    }
    homeScreen();
  }
  //If the card IS registered
  else if(global & 1 << REGISTERED){
    Serial.println(F("Card is already registered"));
  }
}

byte enterCardIndex(){
  //Serial.print(F("Enter card index: "));
  byte cardIndex = 0;
  /*cleanSerial();
  while(!(Serial.available() > 0)){
    ;
  }*/
  byte received[3] = {};
  byte i = 0;
  while(Serial.available()){
    //Serial.flush();
    received[i] = Serial.read();
    Serial.println(received[i]);
    i++;
  }
  //Serial.print(received[1]);
  //Serial.println(F(" after"));
  for(byte b = 0; b < i-2; b++){
    cardIndex += byte(pow(10, (i-b-3)) * (received[b]-48));
  }
  return cardIndex;
}

void viewCards(){ //Loads the card nums from EEPROM
  DISPLAY_NAME.clearDisplay();
  maxOption = cardCount - 1;
  byte cursorY = 2;
  boolean zeroBeginning = false;  //This variable determines whether the card begins with 0xFF (this function could later be removed)
  byte displayLine = 0; //set the display line
  option = 0; //set the cursor to the first index by making the option variable 0
  byte prevOption = 1;
  byte beginIndex = 0;
  byte secondIndex = 0;
  byte belowIndex = 0;
  //Serial.println("Entered a loop");
  while((global & 1 << ADMIN_MENU) && !Serial.available()){
    if(prevOption != option){
      DISPLAY_NAME.fillRect(0,0,8,64,BLACK);  //clean the cursor pixels
      //move the cursor only in the display area
      if(prevOption < option){
        if(cursorY <= 50){
          global &= ~(1 << SCROLL_FLAG);  
          cursorY += 8;
        }
        else if(prevOption != maxOption){
          global |= 1 << SCROLL_FLAG;
        }
      }
      if(prevOption > option){
        if(cursorY > 2){
          global &= ~(1 << SCROLL_FLAG);    
          cursorY -= 8;
        }
        else{
          global |= 1 << SCROLL_FLAG;
        }
      }
      if((global & 1 << SCROLL_FLAG) || option == 0){
        DISPLAY_NAME.clearDisplay();
      }
      DISPLAY_NAME.drawRect(2, cursorY, 4, 4, WHITE); //draw the cursor
      displayLine = 0;
      for(byte i = 0; i < (MAX_EEPROM + 1 - ADMIN_CARDS) / 4; i++){
        for(byte y = 0; y < 4; y++){
          if(EEPROM.read((i*4)+y) == 0xFF){
            if(y == 0){
              zeroBeginning = true;
            }
          }
          else{
            if(y == 0){
              if(global & 1 << SCROLL_FLAG){
                if(prevOption > option){
                  beginIndex = belowIndex;
                }
                if(prevOption < option){
                  beginIndex = secondIndex;
                }
                Serial.println("scroll fl");
                Serial.print("Below ");
                Serial.println(belowIndex);
                global &= ~(1 << SCROLL_FLAG);  
              }
              if(secondIndex <= beginIndex){
                secondIndex = i;
              }
              if(i < beginIndex){
                belowIndex = i;
              }
              else if(i == 0 && beginIndex == 0){
                belowIndex = 0;
              }
              Serial.print(F("Index "));
              Serial.print(i);
              Serial.print(F(": "));
              if(displayLine < 8 && i >= beginIndex){
                printText((char*)"Index ", 8, 8*displayLine, 1, WHITE);
                DISPLAY_NAME.print(i);
                DISPLAY_NAME.print((char*)": ");
                Serial.println("CONDITI");
              }
            }
            if(y > 0 && zeroBeginning){
              Serial.print(F("Card with zero beginning"));
              Serial.print(F("Index "));
              Serial.print(i);
              Serial.print(F(": "));
              for(byte z = 0; z <= y; z++){
                Serial.print(EEPROM.read((i*4)+z), HEX);
                Serial.print(' ');
              }
            }
            if(!zeroBeginning){
              Serial.print(EEPROM.read((i*4)+y), HEX);
              Serial.print(' ');
              /*if(global & 1 << SCROLL_FLAG){
                DISPLAY_NAME.print(EEPROM.read((i*4)+y), HEX);
              }*/
            }
            if(y == 3){
              Serial.println();
              selectedCardIndex = i;
              /*for(byte admin = 0; admin < ADMIN_CARDS; admin++){
                if(EEPROM.read(MAX_EEPROM - ADMIN_CARDS + 1 + admin) == i){
                  printText("A", 120, 8*displayLine, 1, WHITE);
                }
              }*/
              if(displayLine < 8 && beginIndex <= i){
                displayLine++;
              }
            }
          }
        }
      }
      zeroBeginning = false;
      DISPLAY_NAME.display();
      prevOption = option;
    }
  }
  if(Serial.available() > 0){
    selectedCardIndex = enterCardIndex();
  }
}

void viewCards(boolean firstTime){ //Loads the card nums from EEPROM
  maxOption = cardCount - 1;
  boolean zeroBeginning = false;  //This variable determines whether the card begins with 0xFF (this function could later be removed)
  //Serial.println("Entered a loop");
  for(byte i = 0; i < (MAX_EEPROM + 1 - ADMIN_CARDS) / 4; i++){
    for(byte y = 0; y < 4; y++){
      if(EEPROM.read((i*4)+y) == 0xFF){
        if(y == 0){
          zeroBeginning = true;
        }
      }
      else{
        if(y == 0){
          Serial.print(F("Index "));
          Serial.print(i);
          Serial.print(F(": "));
        }
        if(y > 0 && zeroBeginning){
          Serial.print(F("Card with zero beginning"));
          Serial.print(F("Index "));
          Serial.print(i);
          Serial.print(F(": "));
          for(byte z = 0; z <= y; z++){
            Serial.print(EEPROM.read((i*4)+z), HEX);
            Serial.print(' ');
          }
        }
        if(!zeroBeginning){
          Serial.print(EEPROM.read((i*4)+y), HEX);
          Serial.print(' ');
        }
        if(y == 3){
          Serial.println();
        }
      }
    }
  }
  zeroBeginning = false;
}

void deleteCards(byte cardIndex){
  for(int i = MAX_EEPROM + 1 - ADMIN_CARDS; i <= MAX_EEPROM; i++){
    if(EEPROM.read(i) == cardIndex){
      EEPROM.write(i, 0xFF);
    }
  }
  for(byte b = 0; b < 4; b++){
    EEPROM.write(cardIndex * 4 + b, 0xFF);
  }
  getCardInfo();
  viewCards(true);
  homeScreen();
}

void makeCardAdmin(byte cardIndex){
  //cardIndex += 1; //something causes the cardIndex variable to be decreased by 1 after the calculation
  //Serial.println(cardIndex);
  for(int a = MAX_EEPROM - ADMIN_CARDS + 1; a <= MAX_EEPROM; a++){
    if(EEPROM.read(a) == 0xFF){
      EEPROM.write(a, cardIndex);
      break;
    }
    else if(a == MAX_EEPROM){
      Serial.println(F("No more admin card capacity"));
    }
  }
}

void isCardAdmin(){
  if(global & 1 << ADMIN_CARD){
    maxOption = sizeof(adminOptions) / 2 - 1;  //determine the maximum option size
    cleanSerial();
    global &= ~(1 << DIM_FLAG);
    TIMSK1 |= (1 << OCIE1B);
    tone(BUZZER, TONE_LOW, 100);
    clearDimTimer();

    //boolean noChar = false;
    global |= 1 << ADMIN_MENU;
    option = 0;

    Serial.println(F("Administrator card inserted"));
    Serial.println(F("---------------------------"));
    Serial.println(F("Login --0"));
    Serial.println(F("Add a new card --1"));
    Serial.println(F("Make a card admin --2"));
    Serial.println(F("Delete an existing card by index --3"));
    Serial.println(F("View all cards --4"));

    DISPLAY_NAME.fillRect(0,0,128-56,20,BLACK);
    DISPLAY_NAME.fillRoundRect(0,0,64,21,2,WHITE);
    displayText((char*)"Admin", 2, 3, 2, BLACK);

    DISPLAY_NAME.drawRoundRect(0,22,64,20,2,WHITE);
    DISPLAY_NAME.setCursor(3, 24);
    DISPLAY_NAME.setTextColor(WHITE);
    for(byte x = 0; x < sizeof(adminOptions[option]); x++){
      DISPLAY_NAME.print(adminOptions[option][x]);
    }
    DISPLAY_NAME.display();
    byte prevOption = 1;
    uint8_t optionLength = 0;
    //option++; //tentative -> for testing only

    while(!Serial.available() && (global & 1 << ADMIN_MENU)){
      if(prevOption != option){
        //Serial.println("CONDITION");
        optionLength = 0;
        printText(adminOptions[option], 3, 24, 2, WHITE);
        //for loop for determineing the option size (divided with '|')
        while(adminOptions[option][optionLength] != '|'){
          optionLength++;
        }
        prevOption = option;
        for(byte i = 0; i < optionLength; i++){
          Serial.print(adminOptions[option][i]);
        }
        Serial.println();
      }
      if(optionLength >= 5){
        //scroll forward loop
        for(byte i = 0; i <= (optionLength - 5) * 12 && (global & 1 << ADMIN_MENU) && prevOption == option; i++){ //space for 5 letters
          DISPLAY_NAME.fillRect(0,23,68,24,BLACK);
          DISPLAY_NAME.setCursor(3-i, 24);
          DISPLAY_NAME.setTextColor(WHITE);
          for(byte x = 0; x < optionLength; x++){
            DISPLAY_NAME.print(adminOptions[option][x]);
          }
          DISPLAY_NAME.fillRect(62,22,68,20,BLACK);
          DISPLAY_NAME.drawRoundRect(0,22,64,20,2,WHITE);
          DISPLAY_NAME.drawBitmap(72,0,gymkrenLogo,56,56,BLACK, WHITE);
          DISPLAY_NAME.display();
          if(global & 1 << DIM_FLAG){
            DISPLAY_NAME.dim(true);
            TIMSK1 &= ~(1 << OCIE1B);
          }
          else{
            DISPLAY_NAME.dim(false);
          }
        }
        //scroll back loop
        for(byte i = (optionLength - 5) * 12 ; i > 0 && (global & 1 << ADMIN_MENU) && prevOption == option; i -= 3){ //space for 5 letters
          DISPLAY_NAME.fillRect(0,22,68,24,BLACK);
          DISPLAY_NAME.setCursor(3-i, 24);
          DISPLAY_NAME.setTextColor(WHITE);
          for(byte x = 0; x < optionLength; x++){
            DISPLAY_NAME.print(adminOptions[option][x]);
          }
          DISPLAY_NAME.fillRect(62,22,68,20,BLACK);
          DISPLAY_NAME.drawRoundRect(0,22,64,20,2,WHITE);
          DISPLAY_NAME.drawBitmap(72,0,gymkrenLogo,56,56,BLACK, WHITE);
          DISPLAY_NAME.display();
          if(global & 1 << DIM_FLAG){
            DISPLAY_NAME.dim(true);
            TIMSK1 &= ~(1 << OCIE1B);
          }
          else{
            DISPLAY_NAME.dim(false);
          }
        }
      }
    }
    if(Serial.available() > 0){ //in case of serial input
        option = Serial.read() - 48; //Just 1st digit; -48 added because the Serial data are being sent as ASCII characters (0 is 48 in ASCII)
        DISPLAY_NAME.fillRect(0,23,68,24,BLACK);
        DISPLAY_NAME.setCursor(2, 24);
        DISPLAY_NAME.setTextColor(WHITE);
        DISPLAY_NAME.print(adminOptions[option]);
        DISPLAY_NAME.fillRect(62,22,68,20,BLACK);
        DISPLAY_NAME.drawRoundRect(0,22,64,20,2,WHITE);
        DISPLAY_NAME.drawBitmap(72,0,gymkrenLogo,56,56,BLACK, WHITE);
        DISPLAY_NAME.display();
        cleanSerial(); //Added this loop because of ASCII line break command
    }
    global |= 1 << ADMIN_MENU; //prepare for the next menu level
    //Here, code after interrupt (OK_BUTTON) happens
    //Serial.println(option);
    if(global & 1 << ADMIN_CARD){
      switch(option){
        case 0:
          login();
          break;
        case 1: 
          addCard();
          break;
        case 2:
          viewCards();
          makeCardAdmin(selectedCardIndex);
          break;
        case 3:
          viewCards();
          deleteCards(selectedCardIndex);
          break; 
        case 4:
          viewCards();
          homeScreen();
          break;
        default:
          Serial.print("No option for value: ");
          Serial.println(option);
      }
    }
    else{
      logout();
    }
    OCR1B |= 1563; //0.1-second interval
  }
}

void setup() 
{
  Serial.begin(BAUDRATE);   // Initiate a serial communication
  
  global &= ~(1 << ADMIN_MENU);

  DISPLAY_NAME.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  DISPLAY_NAME.clearDisplay();
  DISPLAY_NAME.drawBitmap(36,0,gymkrenLogo,56,56,BLACK, WHITE);
  DISPLAY_NAME.display();

  DISPLAY_NAME.setTextSize(1);
  DISPLAY_NAME.setTextColor(WHITE);
  DISPLAY_NAME.setCursor(0, 56);
  //Admin options
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  //
  eepromPrint();
  //
  Serial.println(MAX_CARDS);
  Serial.println(F("--RFID system--"));
  //
  getCardInfo();
  //Animate the logo
  for(int x = 36; x <= (128-56); x += 4){
    DISPLAY_NAME.fillRect(0, 0, x, 56, BLACK);
    DISPLAY_NAME.drawBitmap(x,0,gymkrenLogo,56,56,BLACK, WHITE);
    DISPLAY_NAME.display();
  }
  //
  Serial.print(F("Saved cards: "));
  Serial.println(cardCount);
  //
  homeScreen(true);

  if(cardCount == 1){
    Serial.println(F("-- Administrator card only"));
  }
  if(cardCount < 1){
    Serial.println(F("no cards loaded"));
  }
  interruptConfig();
  //Load cards from EEPROM to variable
  viewCards(true);
  Serial.println(F("Approximate your card to the reader..."));
  
  displayDimSetup();
  DISPLAY_NAME.dim(false);
  
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
}
void loop(){
  // dim the display after reaching the timer interrupt
  if(global & 1 << DIM_FLAG){
    DISPLAY_NAME.dim(global & 1 << DIM_FLAG);
    TIMSK1 &= ~(1 << OCIE1B);
  }
  else{
    DISPLAY_NAME.dim(global & 1 << DIM_FLAG);
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  mfrc522.PICC_HaltA(); //stop the reading

  DISPLAY_NAME.dim(false);
  clearDimTimer();  //start the dim timer from 0

  Serial.println(F("Actual card number: "));
  //Read the card number:
  getCardNumber();
  //Check if the card is registered:
  isCardRegistered();
  //Check if the inserted card has admin permissions:
  isCardAdmin();
  //
  for(byte b = 0; b < 50; b++){ //divide the readings
    Serial.print('=');
  }
  Serial.println();
  delay(2000);
}