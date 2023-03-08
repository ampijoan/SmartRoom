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
bool emfToggle;

IoTTimer ouijaTimer;
IoTTimer tempTimer;
IoTTimer candleFlickerTimer;
IoTTimer emfTimer;
Encoder spiritEncoder(ENCPINB, ENCPINA);
Adafruit_NeoPixel candlePixel(PIXELCOUNT, PIXELPIN, WS2812B);
Adafruit_BMP280 spiritBmp;
Adafruit_SSD1306 spiritDisplay(OLED_RESET);


// SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  WiFi.on();
  WiFi.setCredentials("IoTNetwork");
  WiFi.connect();

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
  pinMode(SWITCHPIN, INPUT_PULLUP);
  pinMode(EMFPIN, INPUT);

}

void loop() {

  if(digitalRead(SWITCHPIN) == LOW){
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

  if(digitalRead(SWITCHPIN) == HIGH){
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
    ouijaToggle = false;
  }
}

void ouijaIot(){
  int i;

  if(ouijaChar == 0){ //HELLO
    for(i=1; i<7; i++){
      setHue(i, true, HueOrange, 250, 250);
    }
  }

  if(ouijaChar == 11){ //YES for Outlets ON
    for(i=0; i<5; i++){
      switchON(i);
    }
  }

  if(ouijaChar == 13){ //B for BLUE
    for(i=1; i<7; i++){
      setHue(i, true, HueBlue, 250, 250);
    }
  }

  if(ouijaChar == 29){ //R for RED
    for(i=1; i<7; i++){
      setHue(i, true, HueRed, 250, 250);
    }
  }

  if(ouijaChar == 31){ //NO for Outlets OFF
    for(i=0; i<5; i++){
      switchOFF(i);
    }
  }

  if(ouijaChar == 34){ //V for VIOLET
    for(i=1; i<7; i++){
      setHue(i, true, HueViolet, 250, 250);
    }
  }

  if (ouijaChar == 39){ //GOOD BYE
    for(i=1; i<7; i++){
      setHue(i, false, 0, 0, 0);
    }
  }

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
  }

  if(currentTemp <= (previousTemp - 5) && tempToggle && tempTimer.isTimerReady()){
    Serial.printf("TEMP DROP DETECTED\n");
    //turn lights blue and dim
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
  // Serial.printf("%i\n", emfLevel);

  if(emfLevel > 2200){
    emfTimer.startTimer(2000);

    candleFlicker(); //need to give this a timer
    //flicker lights and outlets
  }

  else{
    candlePixel.setBrightness(0);
    candlePixel.show();
    //brighten hue lights back up

  }

}
