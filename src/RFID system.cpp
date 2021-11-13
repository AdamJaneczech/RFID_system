#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
 
#define SS_PIN 10
#define RST_PIN 9
#define BAUDRATE 115200
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

byte cards[][4]{
  {0x77, 0x02, 0x8E, 0x3F},
};

byte actualCard[4];
byte cardCount;
byte adminCards[]{};  //variable for admin card indexes

String cardString[4];
String cardName[] = {};
  
boolean adminCard = false;
boolean registered = false;

void login(){
  Serial.println("Passed. Login OK");
}

void getCardCount(){
  cardCount = 1;
  while(EEPROM.read(4 * cardCount) != 255){ //EEPROM address -> card value has 4 8-bit numbers
    cardCount++;
    delay(500);
    Serial.println("RUNNING");
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
    if(i == 0){
      adminCard = (actualCard[i] == cards[0][i]);
    }
    if(adminCard == true && i != 0){
      adminCard = actualCard[i] == cards[0][i];
    }
  }
}

void isCardRegistered(){
  Serial.println();
  registered = false; //???
  for(int i = 0; i <= cardCount && !registered; i++){
    Serial.println("LOOP STARTED");
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
    if(registered && !adminCard){
      Serial.println("Card registered at the number of " + String(i));
    }
    if(!registered && i != 0){
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
  isCardRegistered();
  //If the card is NOT registered
  if(!registered){
    Serial.println("New card number: ");
    for(int i = 0; i < mfrc522.uid.size; i++){
      cards[cardCount][i] = mfrc522.uid.uidByte[i];
      if(cardCount < 2){
        EEPROM.write(i + 4, cards[cardCount][i]);
      }
      else{
        EEPROM.write(i + 4*(cardCount), cards[cardCount][i]);
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
  if(registered){
    Serial.println("Card is already registered");
  }
}

void deleteCard(byte index){
  Serial.println("deleteCard");
}

void makeCardAdmin(byte index){
  Serial.println("makeCardAdmin");
}

void sortCards(){
  Serial.println("sortCards");
}

void viewCards(){

}

void isCardAdmin(){
  if(adminCard){
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
    char reading;
    while(Serial.available() > 0){  //!!!
      reading = char(Serial.read());
      Serial.println(reading);
    }
    switch(reading){
      case '0':
        break;
      case '1': 
        addCard();
        break;
      case '2':
        makeCardAdmin(0);
        break;
      case '3':
        deleteCard(0);
        break; 
      case '4':
        viewCards();
        break;
      default:
        Serial.print("Selected value: " + String(reading));
    }
  }
}

void setup() 
{
  Serial.begin(BAUDRATE);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  cardName[0] = "AdamJanecek"; //for future card holder names
  
  Serial.println("--RFID system--");
  getCardCount();

  Serial.print("Saved cards: ");
  Serial.println(String(cardCount));
  if(cardCount == 1){
    Serial.println("-- Administrator card only");
  }
  Serial.println("Cards loaded: ");
  if(cardCount <= 1){
    Serial.println("no cards loaded");
  }
  //Load cards from EEPROM to variable
  for(int i = 1; i <= cardCount; i++){
    for(int y = 0; y < 4; y++){
      cards[i][y] = EEPROM.read((i*4)+y);
      Serial.print(cards[i][y], HEX);
    }
    Serial.println();
  }
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