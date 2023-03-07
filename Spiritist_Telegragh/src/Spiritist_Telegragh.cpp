/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegragh/src/Spiritist_Telegragh.ino"
/*
 * Project Spiritist_Telegraph
 * Description: Room controller that allows for spirit communication and allows for spirits to control lights 
 * and outlets in the room.
 * Author: Adrian Pijoan
 * Date: 06-MAR-2023
 */

#include "wire.h"
#include "IoTClassroom_CNM.h"
#include "IoTTimer.h"
#include "encoder.h"
#include "neopixel.h"
#include "Adafruit_BMP280.h"
#include "Adafruit_SSD1306.h"
#include "ouija.h"
#include "bitmaps.h"


void setup();
void loop();
void ouija();
void tempDrop();
void emf();
#line 20 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegragh/src/Spiritist_Telegragh.ino"
const int PIXELPIN = D8;
const int PIXELCOUNT = 2;
const int ENCLEDR = D4;
const int ENCLEDG = D5;
const int ENCSWITCH = D6;
const int ENCPINA = A3;
const int ENCPINB = A4;
const int SWITCHPIN; //needs a pin assigned

const int EMFPIN = A0;
const int BMPADDRESS = 0x76;
const int OLEDADDRESS = 0x3C;
const int OLED_RESET = D4;

int encPosition;
int ouijaChar;
int previousOuijaChar;
bool ouijaToggle;
int currentTemp;
int previousTemp;
int emfLevel;
bool tempToggle;
bool switchState;

IoTTimer ouijaTimer;
IoTTimer tempTimer;
Encoder spiritEncoder(ENCPINB, ENCPINA);
Adafruit_NeoPixel candlePixel(PIXELCOUNT, PIXELPIN, WS2812B);
Adafruit_BMP280 spiritBmp;
Adafruit_SSD1306 spiritDisplay(OLED_RESET);


SYSTEM_MODE(SEMI_AUTOMATIC);
// SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  // WiFi.on();
  // WiFi.setCredentials("IoTNetwork");
  // WiFi.connect();

  Wire.begin();
  spiritDisplay.begin(SSD1306_SWITCHCAPVCC, OLEDADDRESS);
  spiritDisplay.setRotation(0);
  spiritDisplay.clearDisplay();
  spiritDisplay.display();

  spiritBmp.begin(BMPADDRESS);

  candlePixel.begin();
  candlePixel.show();

  pinMode(ENCSWITCH, INPUT_PULLUP);
  pinMode(ENCLEDG, OUTPUT);
  pinMode(ENCLEDR, OUTPUT);
  pinMode(SWITCHPIN, INPUT);
  pinMode(EMFPIN, INPUT);

}

void loop() {

  switchState = digitalRead(SWITCHPIN);
  //run ouija no matter what
  if(switchState = false){
    digitalWrite(ENCLEDG, LOW);
    digitalWrite(ENCLEDR, HIGH);
    ouija();
    currentTemp = (1.8 * spiritBmp.readTemperature())+32;
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(5,54);
    display.printf("T: (%i%c)", currentTemp, 0xF8);
  }
  //if ghost switch is flipped, also read Temp and EMF
  if(switchState){
    digitalWrite(ENCLEDG, HIGH);
    digitalWrite(ENCLEDR, LOW);
    ouija();
    tempDrop();
    emf();
  }


  // if(ouijaChar != previousOuijaChar){ //check to see if planchette has moved to a new position
  //   ouijaTimer.startTimer(500); //wait 0.5 seconds
  //   ouijaToggle = true;
  // }

  // previousOuijaChar = ouijaChar;

  // if((previousOuijaChar == ouijaChar) && ouijaToggle && ouijaTimer.isTimerReady()){  //if it sees a new position and it has held that position for more than 3/4 a second, do something
  //   Serial.printf("%s\n", ouijaBoard[ouijaChar]);
  //   ouijaToggle = false;
  // }
 

}

//my functions

void ouija(){

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
    //send ouijaChar into a function that does things with the wemo and hue...
    ouijaToggle = false;
  }
}

void tempDrop(){
  //if the temp drops by at least 5 degrees F within a matter of 5 seconds, do something (probably dim the lights)
  currentTemp = (1.8 * spiritBmp.readTemperature())+32;//Read the BMP

  if(currentTemp != previousTemp && tempToggle == false){
    tempTimer.startTimer(10000);
    previousTemp = currentTemp;
    tempToggle = true;
  } //need to make sure tempToggle actually does what I think it does and the timer doesn't endlessly reset.

  if(currentTemp <= (previousTemp - 5) && tempToggle && tempTimer.isTimerReady()){
    //do something with the lights...probably change color and dim?
    tempToggle = false;
  }

  if(tempToggle && tempTimer.isTimerReady()){
    tempToggle = false;
  }

}

void emf(){
  //if EMF reading exceeds a certain threshold, flash the lights on and off
  emfLevel = analogRead(EMFPIN);

  if(emfLevel > 2000){ //currently an arbitrary number, need to test for actual range
    //flicker the lights for a set amount of time
    candlePixel.setBrightness(32);
    //also dim hue lights
  }

  else{
    candlePixel.setBrightness(0);
    candlePixel.show();
    //brighten hue lights

  }

}