#include <sha1.h> // From https://github.com/damico/ARDUINO-OATH-TOKEN
#include <SPI.h>
#include <Wire.h>
// Date and time functions using a DS3234 RTC connected via I2C and Wire lib
#include <RTClib.h>
#include <RTC_DS3234.h> // https://github.com/maniacbug/RTClib

// Avoid spurious warnings
#undef PROGMEM
#define PROGMEM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];}))

// Create an RTC instance, using the chip select pin it's connected to
RTC_DS3234 RTC(8);

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<20; i++) Serial.println(hash[i]);
  Serial.println();
}

// Fill this with your own secret as described on http://blog.jfedor.org/2013/02/google-authenticator-watch.html
uint8_t hmacKey1[]={ 0x00, 0x00, 0x00 };

int buttonPin = 2;              // switch is connected to pin 2
int buttonState;                // variable to hold the button state

void setup () {

  pinMode(buttonPin, INPUT);     
  Serial.begin(57600);
  SPI.begin();
  RTC.begin();

  /* The RTC must be set to Greenwich Mean Time (GMT) in order to be in sync with Google’s clock. 
   Set the time on your local machine to GMT and compile+upload the sketch.  When the RTC is set 
   you can comment this line again. */
  //RTC.adjust(DateTime(__DATE__, __TIME__));

  Keyboard.begin();

}

void loop () {

  // Wait for button press
  while(true) {
    buttonState = digitalRead(buttonPin);  
    if(buttonState == HIGH) break; 
  }

  // Read the time from RTC
  DateTime now = RTC.now();

  // Print the time so that we can check if it is correct. Compare with http://www.epochconverter.com/
  Serial.println(now.unixtime()+20); // Add 20 seconds due to delay between compilation and RTC programming

  uint8_t byteArray[8];   
  long time = (now.unixtime()+20) / 30;

  byteArray[0] = 0x00;
  byteArray[1] = 0x00;
  byteArray[2] = 0x00;
  byteArray[3] = 0x00;
  byteArray[4] = (int)((time >> 24) & 0xFF) ;
  byteArray[5] = (int)((time >> 16) & 0xFF) ;
  byteArray[6] = (int)((time >> 8) & 0XFF);
  byteArray[7] = (int)((time & 0XFF));

  uint8_t* hash;
  uint32_t a; 
  Sha1.initHmac(hmacKey1,10);
  Sha1.writebytes(byteArray, 8);
  hash = Sha1.resultHmac();

  int  offset = hash[20 - 1] & 0xF; 
  long truncatedHash = 0;
  int j;
  for (j = 0; j < 4; ++j) {
    truncatedHash <<= 8;
    truncatedHash  |= hash[offset + j];
  }

  truncatedHash &= 0x7FFFFFFF;
  truncatedHash %= 1000000;

  //Serial.println(truncatedHash);
  Keyboard.println(truncatedHash);

  delay(10000);
}

