#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

byte actualCard[4];
byte actualCardIndex;
byte cardCount = 1;
byte lowestEmptyIndex;
byte option = 0;

String cardString[4];
  
boolean adminCard = false;
boolean registered = false;
boolean adminMenu = false;
boolean pressed = false;

char *adminOptions[] = {"Login|", "Add ID|", "Add admin|", "Delete ID|"};

#include <Config.h>

#include <ControlButtons.h>
#include <OLED_icons.h>
#include <DisplayFunctions.h>

void eepromPrint(){
  Serial.println(F("Memory map:"));
  for(int i = 0; i <= MAX_EEPROM; i++){
    Serial.print(EEPROM.read(i));
    Serial.print(' ');
    if(i % 31 == 0 && i != 0){
      Serial.println();
    }
  }
}

void cleanSerial(){
  Serial.end();
  Serial.begin(BAUDRATE);
}

void logout(){
  digitalWrite(RELAY, HIGH);
  homeScreen();
  Serial.println(F("Logout"));
}

void login(){
  digitalWrite(RELAY, LOW);
  Serial.println(F("Passed. Login OK"));
  allowed = true;
  cleanSerial();
  DISPLAY_NAME.fillRect(0,0,128-56,20,BLACK);
  DISPLAY_NAME.fillRoundRect(0,0,72,20,2,WHITE);
  displayText("Active", 2, 2, 2, BLACK);
  while(!Serial.available() > 0 && allowed){
    
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
        Serial.print("Lowest: ");
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
    //Possibly a problem with String() -> out of memory
    cardString[i] = String(actualCard[i], HEX);
    if(cardString[i].length() < 2){
      cardString[i] = "0" + String(actualCard[i], HEX);
    }
    cardString[i].toUpperCase();
    if(actualCard[i] < 0xF){
      Serial.print('0');
      DISPLAY_NAME.print('0');
    }
    Serial.print(actualCard[i], HEX /*+ " "*/);
    DISPLAY_NAME.print(actualCard[i], HEX);
    DISPLAY_NAME.display();
  }
}

void isCardRegistered(){ //modified here -> EEPROM direct reading
  Serial.println();
  registered = false;
  adminCard = false;
  for(int i = 0; i <= MAX_EEPROM + 1 - ADMIN_CARDS * 4 && !registered; i += 4){
    for(int y = i; y < i + 4; y++){
      if(EEPROM.read(y) == actualCard[y-i] && y == i){
        registered = true;
        Serial.println(F("OK"));
      }
      if(EEPROM.read(y) == actualCard[y-i] && y != i && registered){
        Serial.println(F("OK"));
      }
      if(EEPROM.read(y) != actualCard[y-i]){
        registered = false;
        break;
      }
    }
    if(registered){
      actualCardIndex = i/4;
      DISPLAY_NAME.fillRect(0,56,128,8,BLACK);
      DISPLAY_NAME.setCursor(0,56);
      DISPLAY_NAME.print(F("Registered at: " ));
      DISPLAY_NAME.print(actualCardIndex);
      DISPLAY_NAME.display();
      //
      Serial.print(F("Card registered at: "));
      Serial.println(actualCardIndex);
      for(int index = MAX_EEPROM - ADMIN_CARDS + 1; index <= MAX_EEPROM; index++){
        adminCard = EEPROM.read(index) == i/4;
        if(adminCard){
          break;
        }
      }
    }
    if(registered && !adminCard){
      login();
      break;
    }
    if(!registered && i == MAX_EEPROM + 1 - ADMIN_CARDS * 4){
      Serial.println(F("ID not yet registered"));
      DISPLAY_NAME.setCursor(0,56);
      DISPLAY_NAME.print(F("ID not yet registered"));
      DISPLAY_NAME.display();
    }
  }
  //registered = false;
}

void addCard(){
  Serial.println(F("Approximate new card to the reader..."));
  clearDisplayLine(6,2);
  displayText("Approach the new card", 0, 56, 1, WHITE);
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
  //Check if the card is registered
  getCardNumber();
  getCardInfo();
  isCardRegistered();
  //If the card is NOT registered
  if(!registered){  //EEPROM direct reading -> modify here
    Serial.println(F("New card number: "));
    for(int i = 0; i < mfrc522.uid.size; i++){
      if(cardCount < ((MAX_EEPROM + 1) / 4) - ADMIN_CARDS){
        EEPROM.write((lowestEmptyIndex) * 4 + i, mfrc522.uid.uidByte[i]); //5th position in EEPROM determines the admin attribute
      }
      //--Print String card number--
      actualCard[i] = mfrc522.uid.uidByte[i];
      cardString[i] = String(actualCard[i], HEX);
      if(cardString[i].length() < 2){
        cardString[i] = "0" + String(actualCard[i], HEX);
        printText("0", 0, 48);
      }
      cardString[i].toUpperCase();
      Serial.print(cardString[i] + " ");
      printText((actualCard[i], HEX), 0, 48, 1, WHITE);
      //----//
    }
    DISPLAY_NAME.display();
    DISPLAY_NAME.flush();
    Serial.print(F("Card "));
    for(int i = 0; i < 4; i++){
      Serial.println(cardString[i] + " ");
    }
    Serial.println("stored as number " + String(lowestEmptyIndex) + " in the database.");
    cardCount++;
    Serial.println("Actual number of stored cards: " + String(cardCount)); 
    clearDisplayLine(7,1);
    printText("New card index: ", 0, 56, 1, WHITE);
    displayText(lowestEmptyIndex);
    DISPLAY_NAME.flush();
    while(adminMenu){
      ;
    }
  }
  //If the card IS registered
  else if(registered){
    Serial.println(F("Card is already registered"));
  }
}

void viewCards(){
  //Loads the card nums from EEPROM
  boolean zeroBeginning = false;
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
    zeroBeginning = false;
  } 
}

byte enterCardIndex(){
  Serial.print(F("Enter card index: "));
  cleanSerial();
  while(!(Serial.available() > 0)){
    ;
  }
  byte received[3] = {};
  byte i = 0;
  while(Serial.available()){
    //Serial.flush();
    received[i] = Serial.read();
    Serial.println(received[i]);
    i++;
  }
  Serial.print(received[1]);
  Serial.println(F(" after"));
  byte cardIndex = 0;
  for(byte b = 0; b < i-2; b++){
    cardIndex += byte(pow(10, (i-b-3)) * (received[b]-48));
  }
  return cardIndex;
}

void deleteCards(){
  Serial.print(F("Select card index: "));
  viewCards();
  byte cardIndex = enterCardIndex();
  for(int i = MAX_EEPROM + 1 - ADMIN_CARDS; i <= MAX_EEPROM; i++){
    if(EEPROM.read(i) == cardIndex){
      EEPROM.write(i, 0xFF);
    }
  }
  for(byte b = 0; b < 4; b++){
    EEPROM.write(cardIndex * 4 + b, 0xFF);
  }
  getCardInfo();
  viewCards();
}

void makeCardAdmin(){
  //cardIndex += 1; //something causes the cardIndex variable to be decreased by 1 after the calculation
  byte cardIndex = enterCardIndex();
  Serial.println(cardIndex);
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
  if(adminCard){
    cleanSerial();

    boolean noChar = false;
    adminMenu = true;
    option = 0;

    Serial.println(F("Administrator card inserted"));
    Serial.println(F("---------------------------"));
    Serial.println(F("Login --0"));
    Serial.println(F("Add a new card --1"));
    Serial.println(F("Make a card admin --2"));
    Serial.println(F("Delete an existing card by index --3"));
    Serial.println(F("View all cards --4"));

    DISPLAY_NAME.fillRect(0,0,128-56,20,BLACK);
    DISPLAY_NAME.fillRoundRect(0,0,64,20,2,WHITE);
    displayText("Admin", 2, 2, 2, BLACK);

    DISPLAY_NAME.drawRoundRect(0,22,68,20,2,WHITE);
    DISPLAY_NAME.setCursor(2, 22);
    DISPLAY_NAME.setTextColor(WHITE);
    DISPLAY_NAME.print(adminOptions[option]);
    DISPLAY_NAME.display();

    byte prevOption = 1;
    uint8_t optionLength = 0;
    //option++; //tentative -> for testing only

    while(!Serial.available() && adminMenu){
      if(prevOption != option){
        Serial.println("CONDITION");
        optionLength = 0;
        printText(adminOptions[option], 2, 22, 2, WHITE);
        //for loop for determineing the option size (divided with '|')
        while(adminOptions[option][optionLength] != '|'){
          optionLength++;
        }
        prevOption = option;
        Serial.println(optionLength);
      }
      if(optionLength >= 5){
        for(byte i = 0; i <= (optionLength - 5) * 12 && adminMenu && prevOption == option; i++){ //space for 5 letters
          DISPLAY_NAME.fillRect(0,22,68,24,BLACK);
          DISPLAY_NAME.setCursor(2-i, 22);
          DISPLAY_NAME.setTextColor(WHITE);
          for(byte x = 0; x < optionLength; x++){
            DISPLAY_NAME.print(adminOptions[option][x]);
          }
          DISPLAY_NAME.fillRect(62,22,68,20,BLACK);
          DISPLAY_NAME.drawRoundRect(0,22,64,20,2,WHITE);
          DISPLAY_NAME.drawBitmap(72,0,gymkrenLogo,56,56,BLACK, WHITE);
          DISPLAY_NAME.display();
        }
        //LOOP -> add here
      }
      if(Serial.available() > 0){ //in case of serial input
          option = Serial.read() - 48; //Just 1st digit; -48 added because the Serial data are being sent as ASCII characters (0 is 48 in ASCII)
          cleanSerial(); //Added this loop because of ASCII line break command
      }
    }
    adminMenu = true; //prepare for the next menu level
    //Here, code after interrupt (OK_BUTTON) happens
    Serial.println(option);
    if(adminCard){
      switch(option){
        case 0:
          login();
          break;
        case 1: 
          addCard();
          break;
        case 2:
          viewCards();
          makeCardAdmin();
          break;
        case 3:
          viewCards();
          deleteCards();
          break; 
        case 4:
          viewCards();
          break;
        default:
          Serial.print("No option for value: " + String(option));
      }
    }
    else{
      logout();
    }
  }
}

void setup() 
{
  Serial.begin(BAUDRATE);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  DISPLAY_NAME.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  DISPLAY_NAME.clearDisplay();
  DISPLAY_NAME.drawBitmap(36,0,gymkrenLogo,56,56,BLACK, WHITE);
  DISPLAY_NAME.display();

  DISPLAY_NAME.setTextSize(1);
  DISPLAY_NAME.setTextColor(WHITE);
  DISPLAY_NAME.setCursor(0, 56);
  //test scroll text
  /*for(uint8_t u = 0; u <= 9*8; u++){
    DISPLAY_NAME.fillRect(0,0,128,8,BLACK);
    DISPLAY_NAME.setCursor(0-u, 0);
    DISPLAY_NAME.print("test text");
    DISPLAY_NAME.fillRect(32,0,96,8,BLACK);
    DISPLAY_NAME.display();
  }*/
  
  //Admin options
  
  //
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
  //Load cards from EEPROM to variable
  viewCards();
  Serial.println(F("Approximate your card to the reader..."));
  
  interruptConfig();
  displayDimSetup();
  //DISPLAY_NAME.dim(true);

  byte a = 128;
  a = a << 2;
  Serial.print("a: ");
  Serial.println(a);
}
void loop(){
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
  
  Serial.println(F("Actual card number: "));
  //Read the card number:
  getCardNumber();
  //Check if the card is registered:
  isCardRegistered();
  //Check if the inserted card has admin permissions:
  isCardAdmin();
  //
  delay(2000);
}