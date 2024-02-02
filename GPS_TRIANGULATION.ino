#include <sim900.h>
#include <SoftwareSerial.h>
#include <Wire.h>

const int pin_tx = 2;  // Change this to 2
const int pin_rx = 3;  // Change this to 3

SoftwareSerial gprs(pin_tx, pin_rx); // TX, RX

void setup()
{
  Serial.begin(9600);
  sim900_init(&gprs, 9600);

  delay(7000);
  // Check if GPRS is ready and signal is established
  if (isGPRSReady() && isSignalEstablished())
  {
    gprs.println("AT+CGATT=1");
    delay(1000);

    printSerial();

    gprs.println("AT+SAPBR=0,1"); // turn off gprs
    delay(1000);

    gprs.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); // setup gprs
    delay(1000);

    printSerial();

    gprs.println("AT+SAPBR=3,1,\"APN\",\"internet.globe.com.ph\""); // setup apn
    delay(1000);

    printSerial();

    gprs.println("AT+SAPBR=1,1"); // turn on gprs
    delay(1000);

    printSerial();

    gprs.println("AT+SAPBR=2,1"); // config gprs settings
    delay(1000);

    printSerial();
  }
  Serial.println("Setup Finished!");
}

void loop()
{
  Serial.print("GPS READY?: ");
  Serial.println(isGPRSReady());
  Serial.print("ESTABLISHED?: ");
  Serial.println(isSignalEstablished());
  // Check if GPRS is ready and signal is established
  if (isGPRSReady() && isSignalEstablished())
  {
    gprs.println("AT+SAPBR=1,1"); // turn on gprs
    Serial.println("Inside of GPS Triangulation if-statement: ");
    gprs.println("AT+CLBS=1,1"); // triangulation
    delay(1000);

    printSerial();

    gprs.println("AT+SAPBR=0,1"); // turn on off when not needed to save battery
  }
  delay(3000);
}

bool isGPRSReady()
{
  gprs.println("AT+CGATT?");
  delay(100);
  String response = readResponse();
  return response.indexOf("1") != -1;
}

bool isSignalEstablished()
{
  gprs.println("AT+CSQ");
  delay(100);
  String response = readResponse();
  return response.indexOf("+CSQ:") != -1;
}

String readResponse()
{
  String response = "";
  while (gprs.available())
  {
    char c = gprs.read();
    response += c;
  }
  return response;
}

void printSerial() {
  while (gprs.available())
    {
      char c = gprs.read();
      Serial.print(c);
    }
}