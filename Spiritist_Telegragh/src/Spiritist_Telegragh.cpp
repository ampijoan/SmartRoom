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
void candleFlicker();
void ouija();
void ouijaIot();
void tempDrop();
void emf();
#line 20 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegragh/src/Spiritist_Telegragh.ino"
const int PIXELPIN = D2;
const int PIXELCOUNT = 2;
const int ENCLEDR = D4;
const int ENCLEDG = D5;
const int ENCSWITCH = D6;
const int ENCPINA = A3;
const int ENCPINB = A4;
const int SWITCHPIN = D7; 
const int EMFPIN = A0;
const int BMPADDRESS = 0x76;
const int OLEDADDRESS = 0x3C;
const int OLED_RESET = D4;

int encPosition;
int ouijaChar;
int previousOuijaChar;
int currentTemp;
int previousTemp;
int emfLevel;
int candleFlickerState = 0;
bool ouijaToggle;
bool tempToggle;
bool switchState = false;

IoTTimer ouijaTimer;
IoTTimer tempTimer;
IoTTimer candleFlickerTimer;
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

  // switchState = digitalRead(SWITCHPIN);

  if(switchState == false){
    digitalWrite(ENCLEDG, 255);
    digitalWrite(ENCLEDR, 0);

    currentTemp = (1.8 * spiritBmp.readTemperature())+32;
    spiritDisplay.setTextSize(1);
    spiritDisplay.setTextColor(WHITE);
    spiritDisplay.setCursor(5,54);
    spiritDisplay.printf("T: (%i%c)", currentTemp, 0xF8);
    spiritDisplay.display();
    spiritDisplay.fillRect(0,54,128,10,BLACK);
    
    ouija();
    candleFlicker();
  }

  //if ghost switch is flipped, also run tempDrop and check for EMF
  if(switchState){
    digitalWrite(ENCLEDG, 255);
    digitalWrite(ENCLEDR, 126);
    tempDrop();
    ouija();
    emf();
  }

}
 


//my functions

void candleFlicker(){
    candlePixel.setPixelColor(0, 0xFFA500);
    candlePixel.setPixelColor(1, 0xFFA500);

  if(candleFlickerState == 0){
    candleFlickerTimer.startTimer(100);
    candlePixel.setBrightness(32);
    candlePixel.show();
    candleFlickerState = 1;
  }

  if(candleFlickerState == 1 && candleFlickerTimer.isTimerReady()){
    candleFlickerTimer.startTimer(50);
    candlePixel.setBrightness(150);
    candlePixel.show();
    candleFlickerState = 2;
  }

  if(candleFlickerState == 2 && candleFlickerTimer.isTimerReady()){
    candleFlickerTimer.startTimer(150);
    candlePixel.setBrightness(75);
    candlePixel.show();
    candleFlickerState = 3;
  }

  if(candleFlickerState == 3 && candleFlickerTimer.isTimerReady()){
    candleFlickerTimer.startTimer(100);
    candlePixel.setBrightness(160);
    candlePixel.show();
    candleFlickerState = 4;
  }

  if(candleFlickerState == 4 && candleFlickerTimer.isTimerReady())
    candleFlickerState = 0;
}

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

  if((previousOuijaChar == ouijaChar) && ouijaToggle && ouijaTimer.isTimerReady()){  //if it sees a new position and it has held that position for more than 0.5 seconds, do something
    Serial.printf("%s\n", ouijaBoard[ouijaChar]);
    spiritDisplay.clearDisplay();
    spiritDisplay.setTextSize(2);
    spiritDisplay.setTextColor(WHITE);
    spiritDisplay.setCursor(32,20);
    spiritDisplay.printf("%s", ouijaBoard[ouijaChar]);
    spiritDisplay.display();
    ouijaIot();
    //send ouijaChar into a function that does things with the wemo and hue...
    ouijaToggle = false;
  }
}

void ouijaIot(){

}

void tempDrop(){
  //if the temp drops by at least 5 degrees F within a matter of 5 seconds, do something (probably dim the lights)
  currentTemp = (1.8 * spiritBmp.readTemperature())+32;//Read the BMP
  spiritDisplay.setTextSize(1);
  spiritDisplay.setTextColor(WHITE);
  spiritDisplay.setCursor(5,54);
  spiritDisplay.printf("T: (%i%c)", currentTemp, 0xF8); //print temp regardless of whether or not the temp drops
  spiritDisplay.display();
  spiritDisplay.fillRect(0,54,128,10,BLACK);

  if(currentTemp != previousTemp && tempToggle == false){
    tempTimer.startTimer(10000);
    previousTemp = currentTemp;
    tempToggle = true;
  } //need to make sure tempToggle actually does what I think it does and the timer doesn't endlessly reset.

  if(currentTemp <= (previousTemp - 5) && tempToggle && tempTimer.isTimerReady()){
    Serial.printf("TEMP DROP DETECTED\n");
    //do something with the lights...probably change color and dim?
    //start timer and reset lights after temp normalizes
    tempToggle = false;
  }

  if(tempToggle && tempTimer.isTimerReady()){
    tempToggle = false;
  }

}

void emf(){
  //if EMF reading exceeds a certain threshold, turn on neopixel candles and flash the lights on and off
  emfLevel = analogRead(EMFPIN);
  Serial.printf("%i\n", emfLevel);

  if(emfLevel > 2200){
    //flicker the lights for a set amount of time
    candlePixel.setPixelColor(0, 0xFFA500);
    candlePixel.setPixelColor(1, 0xFFA500);
    candlePixel.setBrightness(32);
    candlePixel.show();
    //dim hue lights
  }

  else{
    candlePixel.setBrightness(0);
    candlePixel.show();
    //brighten hue lights

  }

}
