/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegragh/src/Spiritist_Telegragh.ino"
/*
 * Project Spiritist_Telegragh
 * Description: Room controller that allows for spirit communication and allows for spirits to control lights 
 * and outlets in the room.
 * Author: Adrian Pijoan
 * Date: 06-MAR-2023
 */

#include "wire.h"
#include "IoTClassroom_CNM.h"
#include "IoTTimer.h"
#include "encoder.h"
#include "Adafruit_BMP280.h"
#include "Adafruit_SSD1306.h"
#include "ouija.h"
#include "neopixel.h"

void setup();
void loop();
#line 18 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegragh/src/Spiritist_Telegragh.ino"
const int PIXELPIN = D8;
const int PIXELCOUNT = 46;
const int ENCLEDR = D4;
const int ENCLEDG = D5;
const int ENCSWITCH = D6;
const int ENCPINA = A3;
const int ENCPINB = A4;

const int EMFPIN = A0;
const int BMEADDRESS = 0x76;
const int OLEDADDRESS = 0x3C;
const int OLED_RESET = D4;

int i;
int encPosition;
int ouijaChar;
int previousOuijaChar;
bool ouijaToggle;

IoTTimer ouijaTimer;
IoTTimer tempTimer;
Encoder spiritEncoder(ENCPINB, ENCPINA);
Adafruit_NeoPixel pixel(PIXELCOUNT, PIXELPIN, WS2812B);
Adafruit_BMP280 spiritBmp;
Adafruit_SSD1306 display(OLED_RESET);


SYSTEM_MODE(SEMI_AUTOMATIC);
// SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  // WiFi.on();
  // WiFi.setCredentials("IoTNetwork");
  // WiFi.connect();

  pinMode(ENCSWITCH, INPUT_PULLUP);
  pinMode(ENCLEDG, OUTPUT);
  pinMode(ENCLEDR, OUTPUT);

  digitalWrite(ENCLEDG, HIGH);
  digitalWrite(ENCLEDR, LOW);

}

void loop() {

  encPosition = spiritEncoder.read();

  if(encPosition < 0){ 
    spiritEncoder.write(96);
    encPosition = 96;
  }

  if(encPosition > 96){
    spiritEncoder.write(0);
    encPosition = 0;
  }

  ouijaChar = map(encPosition,0,96,0,39); //map encoder position to ouijaBoard array

  if(ouijaChar != previousOuijaChar){ //check to see if planchette has moved to a new position
    ouijaTimer.startTimer(500); //wait 0.5 seconds
    ouijaToggle = true;
  }

  previousOuijaChar = ouijaChar;

  if((previousOuijaChar == ouijaChar) && ouijaToggle && ouijaTimer.isTimerReady()){  //if it sees a new position and it has held that position for more than 3/4 a second, do something
    Serial.printf("%s\n", ouijaBoard[ouijaChar]);
    ouijaToggle = false;
  }
 

}