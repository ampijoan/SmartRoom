/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegraph/src/Spiritist_Telegraph.ino"
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
#line 19 "/Users/adrianpijoan/Documents/IoT/SmartRoom/Spiritist_Telegraph/src/Spiritist_Telegraph.ino"
const int PIXELCOUNT = 2;
const int PIXELPIN = D2;
const int ENCLEDR = D4;
const int ENCLEDG = D5;
const int ENCSWITCH = D6;
const int SWITCHPIN = D7; 
const int ENCPINA = A3;
const int ENCPINB = A4;
const int EMFPIN = A1;
const int BMPADDRESS = 0x76;
const int OLEDADDRESS = 0x3C;
const int OLED_RESET = D3;

int currentTemp;
int previousTemp;

IoTTimer ouijaTimer;
IoTTimer tempTimer;
IoTTimer candleFlickerTimer;
IoTTimer emfTimer;
IoTTimer emfFlashTimer;
Encoder spiritEncoder(ENCPINB, ENCPINA);
Adafruit_NeoPixel candlePixel(PIXELCOUNT, PIXELPIN, WS2812B);
Adafruit_BMP280 spiritBmp;
Adafruit_SSD1306 spiritDisplay(OLED_RESET);

void candleFlicker();
void ouija();
void ouijaIot(int _ouijaChar);
void tempDrop();
void emf();
void emfFlash();

SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  WiFi.on();
  WiFi.setCredentials("IoTNetwork");
  WiFi.connect();

  spiritDisplay.begin(SSD1306_SWITCHCAPVCC, OLEDADDRESS);
  spiritDisplay.setRotation(0);
  spiritDisplay.clearDisplay();
  spiritDisplay.drawBitmap(32,0,ghost_bmp,64,64,WHITE);
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

  digitalWrite(ENCLEDG, 255);

  if(digitalRead(SWITCHPIN)){
    digitalWrite(ENCLEDR, 126);

    tempDrop();
    ouija();
    emf();
  }

  else{
    digitalWrite(ENCLEDR, 0);

    currentTemp = (1.8 * spiritBmp.readTemperature())+32;
    spiritDisplay.setTextSize(1);
    spiritDisplay.setTextColor(WHITE);
    spiritDisplay.setCursor(5,54);
    spiritDisplay.printf("T: (%i%c)", currentTemp, 0xF8);
    spiritDisplay.display();
    spiritDisplay.fillRect(0,54,40,10,BLACK);
    
    ouija();
    candleFlicker();
  }

}


//this function flickers a neopixel inside of a 3D printed candle
void candleFlicker(){

  static int candleFlickerState = 0;

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

  if(candleFlickerState == 4 && candleFlickerTimer.isTimerReady()){
    candleFlickerState = 0;
  }
}

//this function converts the encoder position into a character from the Ouija Array
void ouija(){

  int encPosition;
  int ouijaChar;
  static int previousOuijaChar;
  static bool ouijaToggle;

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

  if(ouijaToggle && ouijaTimer.isTimerReady()){  //if it sees a new position and it has held that position for more than 0.5 seconds, do something
    Serial.printf("%s\n", ouijaBoard[ouijaChar]);
    spiritDisplay.clearDisplay();
    spiritDisplay.setTextSize(2);
    spiritDisplay.setTextColor(WHITE);
    spiritDisplay.setCursor(32,20);
    spiritDisplay.printf("%s", ouijaBoard[ouijaChar]);
    spiritDisplay.display();

    ouijaIot(ouijaChar);//do something with the ouija character...in this case interact with the IoT classroom, but change this out next time you use this code

    ouijaToggle = false;
  }
}

//this function uses the Ouija Board reading to alter the states of hue lights and wemo outlets in the IoT Classroom
void ouijaIot(int _ouijaChar){

  int i;

  if(_ouijaChar == 0){ //HELLO
    for(i=1; i<7; i++){
      setHue(i, true, HueOrange, 250, 250);
    }
  }

  if(_ouijaChar == 11){ //YES for Outlets ON
    for(i=0; i<5; i++){
      switchON(i);
    }
  }

  if(_ouijaChar == 13){ //B for BLUE
    for(i=1; i<7; i++){
      setHue(i, true, HueBlue, 250, 250);
    }
  }

  if(_ouijaChar == 18){ //G for GREEN
    for(i=1; i<7; i++){
      setHue(i, true, HueGreen, 250, 250);
    }
  }

  if(_ouijaChar == 29){ //R for RED
    for(i=1; i<7; i++){
      setHue(i, true, HueRed, 250, 250);
    }
  }

  if(_ouijaChar == 31){ //NO for Outlets OFF
    for(i=0; i<5; i++){
      switchOFF(i);
    }
  }

  if(_ouijaChar == 34){ //V for VIOLET
    for(i=1; i<7; i++){
      setHue(i, true, HueViolet, 250, 250);
    }
  }

  if (_ouijaChar == 39){ //GOOD BYE
    for(i=1; i<7; i++){
      setHue(i, false, 0, 0, 0);
    }
  }

}

//This function looks for a temperature drop
void tempDrop(){

  int i;
  static bool tempToggle;
  //if the temp drops by at least 5 degrees F within a matter of 10 seconds, do something
  currentTemp = (1.8 * spiritBmp.readTemperature())+32;//Read the BMP
  spiritDisplay.setTextSize(1);
  spiritDisplay.setTextColor(WHITE);
  spiritDisplay.setCursor(5,54);
  spiritDisplay.printf("T: (%i%c)", currentTemp, 0xF8); //print temp regardless of whether or not the temp drops
  spiritDisplay.display();
  spiritDisplay.fillRect(0,54,40,10,BLACK);

  if(currentTemp != previousTemp && tempToggle == false){
    tempTimer.startTimer(10000);
    previousTemp = currentTemp;
    tempToggle = true;
  }

  if(currentTemp <= (previousTemp - 5) && tempToggle && tempTimer.isTimerReady()){
    //once cold spot is detected, turn lights blue and gradually dim them
    Serial.printf("TEMP DROP DETECTED\n");
    spiritDisplay.clearDisplay();
    spiritDisplay.drawBitmap(32,0,ghost_bmp,64,64,WHITE);
    spiritDisplay.display();
    for(i=1; i<7;i++){
      setHue(i, true, HueBlue, 50, 250);
    }
    tempToggle = false;
  }

  if(tempToggle && tempTimer.isTimerReady()){
    tempToggle = false;
  }

}

//This function reads the EMF meter
void emf(){

  int i;
  static bool emfToggle;
  //if EMF reading exceeds a certain threshold, turn on neopixel candles and flash the lights on and off
  int emfLevel = analogRead(EMFPIN);
  Serial.printf("%i\n", emfLevel);

  if(emfLevel > 2000 && emfToggle == false){
    candlePixel.clear();
    candlePixel.setBrightness(150);
    candlePixel.show();
    emfTimer.startTimer(4000);
    spiritDisplay.clearDisplay();
    spiritDisplay.drawBitmap(32,0,ghost_bmp,64,64,WHITE);
    spiritDisplay.display();
    emfToggle = true;
  }

  if(emfToggle){
    emfFlash();
  }

  if(emfToggle && emfTimer.isTimerReady()){
    candlePixel.clear();
    candlePixel.setBrightness(0);
    candlePixel.show();
    for(i=0;i<6;i++){
      switchOFF(i);
      setHue(i+1, false, 0, 0, 0);
      emfToggle = false;
    }
  }

}

//This function changes states of Hue Lights and Wemo Outlets in the IoT Classroom based on readings from the EMF meter
void emfFlash(){ //flash Hue lights and Wemos on and off once every half second

  int i;
  static bool emfFlashToggle = true;

  if(emfFlashToggle){
    emfFlashTimer.startTimer(500);
    emfFlashToggle = false;
  }

  if(emfFlashTimer.isTimerReady()){

    for(i=0; i<6; i++){
      switchOFF(i);
      setHue(i+1, false, 0, 0, 0);
    }
    emfFlashToggle = true;
  }

  else{
    for(i=0; i<6; i++){
      switchON(i);
      setHue(i+1, true, HueOrange, 250, 250);
    }
  }

}

