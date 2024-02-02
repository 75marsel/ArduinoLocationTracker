/**
  GPS SMS TRACKER - ARDUINO UNO
  NAME: GPS_SMS_TRACKER
  FUNCIONALITY: To locate current location using NEO6Mv2 and SIM800L. Short range tracker using Buzzer and nRF24L01.

  @author Gappi, Jeric Marcel L.
  @version 2 build 181223

  This module utilizes SIM800, GPS NEO6Mv2 and nRF24L01
*/


#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <GPRS_Shield_Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_TX    2
#define PIN_RX    3
#define BUZZER_PIN 8
#define BAUDRATE  9600

RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";

char MESSAGE[50];
char phoneNumber[14];
char currentSender[14];
char datetime[24];

bool validLoc;

String msg;
char temp[51]; // 45 characters, 50 for free + 1 null

uint8_t button_state = 0;
uint8_t prev = 0;

unsigned long previousMillis = 0;

TinyGPSPlus gps;
GPRS GSMTEST(PIN_TX, PIN_RX, 9600);


void setup() {
  Serial.begin(9600);
  Serial.print("begin: ");
  bool rad = radio.begin();

  Serial.println(rad);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  pinMode(BUZZER_PIN, OUTPUT);
  while(!GSMTEST.init()) {
    Serial.print(F("INIT ERROR\r\n"));
    readButton();
    delay(50);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  while (Serial.available() > 0) 
  if (gps.encode(Serial.read())) {
    displayInfo();
    readButton();
    if(currentMillis - previousMillis >= 3000) {
      previousMillis = currentMillis;
      readSMS();
    }
    readButton();
  }
  
  readButton();
  delay(10);
  //readButton();
}

// Parse the current SMS Message and perform specific actions depending on the SMS Message.
void sendLocation() {
  char combinedArray[50];  // Character array to store the combined values
  int totalLength = 0;

  if(MESSAGE[0] != '\0') {

    // Sends the GPS Location
    if(strncmp(MESSAGE, "LOCATE", sizeof(MESSAGE)) == 0) { // && validLoc && strcmp(currentSender, phoneNumber) == 0
      totalLength += sprintf(combinedArray + totalLength, "Please copy-paste this to any Maps Software you use:\n");
      totalLength += sprintf(combinedArray + totalLength, temp);
      combinedArray[totalLength] = '\0';
      delay(450);
      //Serial.println(combinedArray);
      Serial.print(F("MSG SENT?: "));
      Serial.println(GSMTEST.sendSMS(currentSender, combinedArray));
      delay(1450);
      //asm volatile ("jmp 0");
    }
    else if(strncmp(MESSAGE, "BEEP", sizeof(MESSAGE)) == 0) { // Alarm the buzzer
      delay(10);
      alarmingSound();
      delay(20);
    }

  currentSender[0] = '\0';
  MESSAGE[0] = '\0';
  readButton();
  //delay(50);
  }
}

// Updates the temp variable (COORDINATES)
void displayInfo() {
  if (gps.location.isValid()) {

    Serial.print(F("Location: "));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.println(gps.location.lng(), 6);

    //msg = "maps.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    msg = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6); //lat, long only
    msg.toCharArray(temp, sizeof(temp));
    Serial.println(temp);
  } 
  else {
    Serial.println(F("Invalid Location"));
    readButton();
  }
  //displayDate();
}

// reads sms
void readSMS() {
  readButton();
  uint8_t msgIndex = GSMTEST.isSMSunread();
  Serial.print(F("Current Index: "));
  Serial.println(msgIndex);
  if(msgIndex > 0 && msgIndex < 255) {
      GSMTEST.readSMS(msgIndex, MESSAGE, 50, currentSender, datetime);
      delay(500);
      GSMTEST.deleteSMS(msgIndex);
      sendLocation();
  }
}
/**
void displayDate() {
if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.println(gps.date.year());
  }
}*/

// Produces a sound from buzzer
void alarmingSound() {
  for (int i = 200; i <= 1000; i += 100) {
    tone(BUZZER_PIN, i);
    delay(50);
  }

  // Falling tones
  for (int i = 1000; i >= 200; i -= 100) {
    tone(BUZZER_PIN, i);
    delay(50);
  }

  noTone(BUZZER_PIN);  // Turn off the buzzer
  delay(500);          // Pause for 1 second between repeats
}

// Reads the button from nRF24L01 sender
void readButton() {
  if (radio.available()) {
    radio.read(&button_state, sizeof(button_state));
  }
  delay(50);
  if (button_state && prev != button_state) {
    delay(10);
    alarmingSound();
  }
  prev = button_state;
}