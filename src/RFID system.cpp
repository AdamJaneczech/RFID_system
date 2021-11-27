#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define BAUDRATE 115200
#define RELAY 7
#define ADMIN_CARDS 8
#define MAX_EEPROM 1023

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

byte cards[256][4]{
  {0x77, 0x02, 0x8E, 0x3F},
};

byte actualCard[4];
byte cardCount = 1;
byte adminCards[ADMIN_CARDS]{};  //variable for admin card indexes
byte lowestEmptyIndex;

String cardString[4];
//String cardName[] = {};
  
boolean adminCard = false;
boolean registered = false;

void cleanSerial(){
  Serial.end();
  Serial.begin(BAUDRATE);
}

void logout(){
  digitalWrite(RELAY, HIGH);
  Serial.println(F("Logout"));
}

void login(){
  digitalWrite(RELAY, LOW);
  Serial.println(F("Passed. Login OK"));
  cleanSerial();
  while(!Serial.available() > 0){
    
  }
  logout();
}

void getCardInfo(){
  cardCount = 0;
  int loopCount = 0;
  byte emptyFields = 0;
  byte emptyIndex = 0;
  Serial.println(F("Empty indexes:"));
  while(loopCount <= MAX_EEPROM + 1 - ADMIN_CARDS * 8){ //EEPROM address -> card value has 4 8-bit numbers
    if(EEPROM.read(loopCount) == 0xFF){
      emptyFields++;
    }
    else if((loopCount + 1) % 4 == 0 && emptyFields < 4){ //a 4 loop checking variable/something needs to be added
      cardCount++;
      if(emptyIndex != 0){
        emptyFields = EEPROM.read(loopCount); //temporarily use the emptyFields variable as a cache
      }
    }
    if(emptyFields == 4){
      if(emptyIndex == 0){
        lowestEmptyIndex = (loopCount + 1) / 4;
        Serial.print("Lowest: ");
      }
      emptyIndex = (loopCount + 1) / 4;
      Serial.println(emptyIndex);
      //Serial.println(", ");
      emptyFields = 0;    
    }
    loopCount++;
  }
}

void getCardNumber(){
  for(int i = 0; i < mfrc522.uid.size; i++){
    actualCard[i] = mfrc522.uid.uidByte[i];
    cardString[i] = String(actualCard[i], HEX);
    if(cardString[i].length() < 2){
      cardString[i] = "0" + String(actualCard[i], HEX);
    }
    cardString[i].toUpperCase();
    Serial.print(cardString[i] + " ");
  }
}

void isCardRegistered(){ //modified here -> EEPROM direct reading
  Serial.println();
  registered = false;
  adminCard = false;
  for(int i = 0; i <= MAX_EEPROM + 1 - ADMIN_CARDS * 8 && !registered; i += 4){
    for(int y = i; y < i + 4; y++){
      if(EEPROM.read(y) == actualCard[y-i] && y == i){
        registered = true;
        Serial.println("Byte " + String(y) + " OK");
      }
      if(EEPROM.read(y) == actualCard[y-i] && y != i && registered){
        Serial.println("Byte " + String(y) + " OK");
      }
      if(EEPROM.read(y) != actualCard[y-i]){
        registered = false;
        break;
      }
    }
    if(registered){
      Serial.println("Card registered at the number of " + String(i));
      //Check for admin cards
      for(int index = 0; index <= cardCount; index++){
        adminCard = (i == adminCards[index]);
        if(adminCard){
          break;
        }
      }
    }
    if(registered && !adminCard){
      login();
      break;
    }
    if(!registered /*&& i == cardCount*/){
      Serial.println(F("Card not registered"));
    }
  }
  //registered = false;
}

void addCard(){
  Serial.println(F("Approximate new card to the reader..."));
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
      if(cardCount < 240){  //MODIFY
        EEPROM.write(lowestEmptyIndex * 4 + i, mfrc522.uid.uidByte[i]); //5th position in EEPROM determines the admin attribute
      }
      //--Print String card number--
      actualCard[i] = mfrc522.uid.uidByte[i];
      cardString[i] = String(actualCard[i], HEX);
      if(cardString[i].length() < 2){
        cardString[i] = "0" + String(actualCard[i], HEX);
      }
      cardString[i].toUpperCase();
      Serial.print(cardString[i] + " ");
      //----//
    }
    Serial.print(F("Card "));
    for(int i = 0; i < 4; i++){
      Serial.println(cardString[i] + " ");
    }
    Serial.println("stored as number " + String(lowestEmptyIndex) + " in the database.");
    cardCount++;
    Serial.println("Actual number of stored cards: " + String(cardCount)); 
  }
  //If the card IS registered
  else if(registered){
    Serial.println(F("Card is already registered"));
  }
}

void viewCards(){
  //Loads the card nums from EEPROM
  byte adminCardCount = 0;
  //the following cycle expects the cards are already ordered (thus the max cardCount value for i -> not searching in the entire memory)
  for(byte i = 0; i < cardCount; i++){
    Serial.print("Index " + String(i) + ": ");
    /*if(EEPROM.read(i) == 0x01){
      adminCards[adminCardCount] = i;
      adminCardCount++;
    }*/
    for(byte y = 0; y < 4; y++){
      Serial.print(EEPROM.read((i*4)+y), HEX);
    }
    Serial.println();
  } 
}

byte enterCardIndex(){
  Serial.print(F("Enter card index: "));
  cleanSerial();
  while(!(Serial.available() > 0)){
    ;
  }
  byte received[5] = {};
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

void sortCards(){
  Serial.println(F("sortCards"));
}

void deleteCards(){
  Serial.print(F("Select card index: "));
  viewCards();
  byte cardIndex = enterCardIndex();
  for(byte b = 0; b < 5; b++){
    EEPROM.write(cardIndex * 5 + b, 0xFF);
    cards[cardIndex][b] = 0;
  }
  getCardInfo();
  viewCards();
  sortCards();
}

void makeCardAdmin(){
  //cardIndex += 1; //something causes the cardIndex variable to be decreased by 1 after the calculation
  byte cardIndex = enterCardIndex();
  boolean zeroDetected = false;
  Serial.println(cardIndex);
  for(byte a = 0; a < sizeof(adminCards); a++){
    if(adminCards[a] == 0){
      zeroDetected = true;
    }
    if(adminCards[a] == 0 && zeroDetected){
      EEPROM.write(cardIndex * 5, 1);
      viewCards();
      break;
    }
  }
  //adminCards[] = cardIndex;
  //Serial.println(cardIndex);
  //Serial.println("makeCardAdmin");
}

void isCardAdmin(){
  if(adminCard){
    cleanSerial();

    Serial.println(F("Administrator card inserted"));
    Serial.println(F("---------------------------"));
    Serial.println(F("Login --0"));
    Serial.println(F("Add a new card --1"));
    Serial.println(F("Make a card admin --2"));
    Serial.println(F("Delete an existing card by index --3"));
    Serial.println(F("View all cards --4"));
    delay(10);
    while(!Serial.available()){
      ;
    }
    byte reading;
    while(Serial.available() > 0){  //!!!
      reading = Serial.read() - 48; //Just 1 digit; -48 added because the Serial data are being sent as ASCII characters (0 is 48 in ASCII)
    }

    cleanSerial(); //Added this loop because of ASCII line break command

    Serial.println(reading);
    //byte readByte = reading.toInt();
    switch(reading){
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
        Serial.print("No option for value: " + String(reading));
    }
  }
}

void setup() 
{
  Serial.begin(BAUDRATE);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  //cardName[0] = "AdamJanecek"; //for future card holder names
  
  Serial.println(F("--RFID system--"));
  getCardInfo();

  Serial.print(F("Saved cards: "));
  Serial.println(cardCount);
  if(cardCount == 1){
    Serial.println(F("-- Administrator card only"));
  }
  Serial.println(F("Cards loaded: "));
  if(cardCount < 1){
    Serial.println(F("no cards loaded"));
  }
  //Load cards from EEPROM to variable
  viewCards();

  Serial.println();
  Serial.println(F("Approximate your card to the reader..."));
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
  delay(3000);
}