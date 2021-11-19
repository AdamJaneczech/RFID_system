#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define BAUDRATE 115200
#define RELAY 7

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

byte cards[][4]{
  {0x77, 0x02, 0x8E, 0x3F},
};

byte actualCard[4];
byte cardCount;
byte adminCards[8]{};  //variable for admin card indexes

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
  Serial.println("Logout");
}

void login(){
  digitalWrite(RELAY, LOW);
  Serial.println("Passed. Login OK");
  cleanSerial();
  while(!Serial.available() > 0){
    
  }
  logout();
}

void getCardCount(){
  cardCount = 1;
  while(EEPROM.read(5 * cardCount + 1) != 255){ //EEPROM address -> card value has 4 8-bit numbers
    cardCount++;
    delay(500);
    //Serial.println("RUNNING");
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
    /*if(i == 0){
      adminCard = (actualCard[i] == cards[0][i]);
    }
    if(adminCard == true && i != 0){
      adminCard = actualCard[i] == cards[0][i];
    }*/
  }
}

void isCardRegistered(){
  Serial.println();
  registered = false;
  adminCard = false;
  for(int i = 0; i <= cardCount && !registered; i++){
    //Serial.println("LOOP STARTED");
    for(int y = 0; y < mfrc522.uid.size; y++){
      if(cards[i][y] == actualCard[y] && y == 0){
        registered = true;
        Serial.println("Byte " + String(y) + " OK");
      }
      if(cards[i][y] == actualCard[y] && y != 0 && registered){
        Serial.println("Byte " + String(y) + " OK");
      }
      if(cards[i][y] != actualCard[y]){
        registered = false;
      }
    }
    if(registered /*&& !adminCard*/){
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
    if(!registered && i == cardCount){
      Serial.println("Card not registered");
    }
  }
  //registered = false;
}

void addCard(){
  Serial.println("Approximate new card to the reader...");
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
  isCardRegistered();
  //If the card is NOT registered
  if(!registered){
    Serial.println("New card number: ");
    for(int i = 0; i < mfrc522.uid.size; i++){
      cards[cardCount][i] = mfrc522.uid.uidByte[i];
      if(cardCount < 2){
        EEPROM.write(i + 6, cards[cardCount][i]); //5th position in EEPROM determines the admin attribute
      }
      else{
        EEPROM.write(i + 5*(cardCount) + 1, cards[cardCount][i]); //formula to avoid writing to every 5th position in EEPROM -> admin attribute
      }
      actualCard[i] = mfrc522.uid.uidByte[i];
      cardString[i] = String(actualCard[i], HEX);
      if(cardString[i].length() < 2){
        cardString[i] = "0" + String(actualCard[i], HEX);
      }
      cardString[i].toUpperCase();
      Serial.println(cardString[i] + " ");
    }
    Serial.print("Card ");
    for(int i = 0; i < 4; i++){
      Serial.print(String(cards[cardCount][i], HEX) + " ");
    }
    Serial.println("stored as number " + String(cardCount) + " in the database.");
    cardCount++;
    Serial.println("Actual number of stored cards: " + String(cardCount)); 
  }
  //If the card IS registered
  else if(registered){
    Serial.println("Card is already registered");
  }
}

void viewCards(){
  //Loads the card nums from EEPROM
  byte adminCardCount = 0;
  for(int i = 0; i < cardCount; i++){
    Serial.print("Index " + String(i) + ": ");
    if(EEPROM.read(i*5) == 0x01){
      adminCards[adminCardCount] = i;
      adminCardCount++;
    }
    for(int y = 1; y <= 4; y++){
      cards[i][y-1] = EEPROM.read((i*5)+y);
      Serial.print(cards[i][y-1], HEX);
    }
    Serial.println();
  } 
}

byte enterCardIndex(){
  Serial.print("Enter card index: ");
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
  Serial.println(" after");
  byte cardIndex = 0;
  for(byte b = 0; b < i-2; b++){
    cardIndex += byte(pow(10, (i-b-3)) * (received[b]-48));
  }
  return cardIndex;
}

void sortCards(){
  Serial.println("sortCards");
}

void deleteCards(){
  Serial.print("Select card index: ");
  viewCards();
  byte cardIndex = enterCardIndex();
  for(byte b = 0; b < 5; b++){
    EEPROM.write(cardIndex * 5 + b, 0xFF);
    cards[cardIndex][b] = 0;
  }
  getCardCount();
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

    Serial.println("Administrator card inserted");
    Serial.println("---------------------------");
    Serial.println("Login --0");
    Serial.println("Add a new card --1");
    Serial.println("Make a card admin --2");
    Serial.println("Delete an existing card by index --3");
    Serial.println("View all cards --4");
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
  
  Serial.println("--RFID system--");
  getCardCount();

  Serial.print("Saved cards: ");
  Serial.println(String(cardCount));
  if(cardCount == 1){
    Serial.println("-- Administrator card only");
  }
  Serial.println("Cards loaded: ");
  if(cardCount < 1){
    Serial.println("no cards loaded");
  }
  //Load cards from EEPROM to variable
  viewCards();

  Serial.println();
  Serial.println("Approximate your card to the reader...");
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
  
  Serial.println("Actual card number: ");
  //Read the card number:
  getCardNumber();
  //Check if the card is registered:
  isCardRegistered();
  //Check if the inserted card has admin permissions:
  isCardAdmin();
  //
  delay(3000);
}