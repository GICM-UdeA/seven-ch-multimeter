//Complete test succesful with and without battery.
//This version implements the diode meter, ohm meter and voltage meter.
//A 1MOhm was soldered between the OPAMP input and GND.
//Changes to V1-3 version: LUT is implemented to calculate the voltage associated with each ADC reading.
//To program: select board within Arduino IDE: Tools->Board->ESP32 Arduino->ESP32 Dev MOdule (if not visible, install ESP32 compatibility first).

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "logo_udea.c"
#include "LUT_ADC.c"
#include "limits.c"

// Definición de constantes para el tamaño de la pantalla
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 135;

//Pin definition for relays
#define RL1 2
#define RL2 13
#define RL3 15
#define RL4 12
#define RL5 33
#define RL6 25
#define RL7 32

//Pindefinition for analog inputs
#define AIN1 27
#define AIN2 26

//Pin definition for buttons
#define button_left 39//39  //35
#define button_right 38//38  //0

//Pin definition for TFT display
#define TFT_CS  5
#define  TFT_DC 16
#define  TFT_MOSI 19
#define  TFT_SCLK 18
#define  TFT_RST  23 
#define  TFT_BACKLIGHT  4
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Color definition
#define TFT_BACKGROUND 0x444a43
#define TFT_TEXT 0xecf5eb
#define TFT_HIGHLIGHT 0x24fa0c
#define TFT_symbol 0x0af5f5
#define TFT_passed 0xff8f00
#define TFT_failed 0x24fa00

// Variables for menu control
unsigned long button_time = 0; //Time when the left button is pressed (for debouncing)
int selectedOption = 0;
int enterOption = false;
int showMenu = true;
const int numOptions = 4;
const char *options[numOptions]={"Ohmeter  ", "Diode    ", "Voltage  ", "Info     "};

//Variables for measurement:
float sum=0; //Sum of the voltages used to calculate the average
float vref=3288; //Reference voltage for ADC in mV
float rs= 1.096; //Series resistor in kOhm
/////////////////////////////////////////////////////////
void setup() {
  //Interrupts for push buttons
  pinMode(button_left, INPUT); //EXTERNAL PULLUP
  pinMode(button_right, INPUT);

  //Relay initialization
  pinMode(RL1, OUTPUT); pinMode(RL2, OUTPUT); pinMode(RL3, OUTPUT); pinMode(RL4, OUTPUT); pinMode(RL5, OUTPUT); pinMode(RL6, OUTPUT); pinMode(RL7, OUTPUT);
  digitalWrite(RL1, 0); digitalWrite(RL2, 0); digitalWrite(RL3, 0); digitalWrite(RL4, 0); digitalWrite(RL5, 0); digitalWrite(RL6, 0); digitalWrite(RL7, 0);

  //Display initialization
  tft.init(SCREEN_HEIGHT,SCREEN_WIDTH);
  tft.setRotation(0); // //0: portrait (usb on top), 2: portrait (usb on bottom), 1: landscape (usb to the right), 3: landscape (usb to the left),
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
  tft.fillScreen(TFT_BACKGROUND);
  tft.drawRGBBitmap(0, 0, logo_udea,  SCREEN_HEIGHT, SCREEN_WIDTH);
  delay(2000);
  tft.fillScreen(TFT_BACKGROUND);
  drawMenu();
  Serial.begin(115200); //Debbuging
}

void loop() {
  if(showMenu==true){
    checkInput(); //Update menu with selected option and go to selected option if enterOption==true
    if((digitalRead(button_left)==0) or (digitalRead(button_right)==0)){  //Update selected option
      delay(50);
      if(digitalRead(button_left)==0){
        if(selectedOption==3){
           selectedOption=0;
         }else{
           selectedOption++;
        }
      }else if(digitalRead(button_right)==0){
        enterOption=true;
        Serial.println("RIGHT");
      }
    }
  }
}

/////////////////////////////MENU FUNCTIONS
void checkInput() {
  //Checks the user input and changes selectedOption accordingly
  drawMenu();
  if(enterOption==true){
    switch (selectedOption) {
      case 0:
        executeOhmeter(); 
        break;
      case 1:
        executeDiode();
        break;
      case 2:
        executeVoltage();
        break;
      case 3:
        executeInfo(); 
        break;
      default:
        break;
    }
  }
}

void drawMenu() {
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(2);

  for (int i = 0; i < numOptions; i++) {
    if (i == selectedOption) {
      tft.setTextColor(TFT_HIGHLIGHT);
    } else {
      tft.setTextColor(TFT_TEXT);
    }

    tft.setCursor(20, 30 + i * 30);
    tft.print(options[i]);
  }
  //Draw function icons
  tft.drawFastHLine(0, 205, 135, TFT_symbol);
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  tft.setCursor(4, 210);
  tft.write(0xEF); //Check map in https://learn.adafruit.com/assets/103682
  tft.setCursor(110, 210);
  tft.write(0x1F);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_TEXT);
}

/////////////////////////////////////////////////////////////////////////////////FUNCTIONS
////////////////////////////////////////////////////////////////////OHMETER
void executeOhmeter() {
  // Displays Ohmeter menu
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(2);
  //Draw lines and boxes
  tft.drawFastHLine(0, 87, SCREEN_HEIGHT, TFT_HIGHLIGHT);
  tft.drawRect(0, 66, SCREEN_HEIGHT, 88, TFT_HIGHLIGHT);
  //Draw text results
  tft.setCursor(25, 25);
  tft.print("OHMETER");
  tft.setCursor(10, 50);
  tft.print("Ref: DRAIN");
  tft.setCursor(10, 70);
  tft.print("Pin");
  tft.write(0x23);
  tft.print("   k");
  tft.write(0xE9);
  tft.setCursor(10, 90);
  tft.print("9: "); //Red
  tft.setCursor(10, 110);
  tft.println("11: "); //White
  tft.setCursor(10, 130);
  tft.println("12: "); // Yellow
  //Draw menu icons
  tft.drawFastHLine(0, 205, 135, TFT_symbol);
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  tft.setCursor(110, 210);
  tft.write(0x1B); //Check map in https://learn.adafruit.com/assets/103682  
  tft.setTextSize(2);
  tft.setTextColor(TFT_TEXT);
  //de-activate showmenu
  showMenu=false;

  digitalWrite(RL7, 1);
  //Pin9 RED
  digitalWrite(RL6, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)];
  }
  float average=sum; //mV (1000/1000)
  float resistance9 = average*rs/(vref-average); //Resistance calculation.
  if(analogRead(AIN1)==0){
    tft.setCursor(50,90);
    tft.write(0xF2);
    tft.setCursor(63,90);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(50,90);
    tft.write(0xF1);
    tft.setCursor(63,90);
  }else{
    tft.setCursor(50,90);
  }
  tft.println(String(resistance9));
  
  //Pin 11 WHITE
  digitalWrite(RL3, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)];
  }
  average=sum;
  float resistance11 = average*rs/(vref-average); //Resistance calculation.
  if(analogRead(AIN1)==0){
    tft.setCursor(50,110);
    tft.write(0xF2);
    tft.setCursor(63,110);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(50,110);
    tft.write(0xF1);
    tft.setCursor(63,110);
  }else{
    tft.setCursor(50,110);
  }
  tft.println(String(resistance11));

    //Pin 12 YELLOW
  digitalWrite(RL3, 0);
  digitalWrite(RL4, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)];
  }
  average=sum;
  float resistance12 = average*rs/(vref-average); //Resistance calculation.
  if(analogRead(AIN1)==0){
    tft.setCursor(50,130);
    tft.write(0xF2);
    tft.setCursor(63,130);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(50,130);
    tft.write(0xF1);
    tft.setCursor(63,130);
  }else{
    tft.setCursor(50,130);
  }
  tft.println(String(resistance12));

  //Process the results
  int result = 0;
  if(inRange(resistance9,res_low_limit,res_high_limit) && inRange(resistance11,res_low_limit,res_high_limit) && inRange(resistance12,res_low_limit,res_high_limit)){
    result = 1; //1 passed, 0 failed.
  }else{
    result= 0;
  }
  //Draw results circle
  if(result==1){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_passed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_passed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("PASS");
  }else if(result==0){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_failed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_failed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("FAIL");
  }
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  digitalWrite(RL6, 0);
  delay(100);
  digitalWrite(RL4, 0);
  digitalWrite(RL7, 0);

  while(digitalRead(button_right)==1); //Waits while right button is not pressed
  delay(500); //If right button is pressed waits for 500ms (debouncing)

  //Restore variables
  showMenu=true;
  enterOption=false;
  tft.fillScreen(TFT_BACKGROUND);
  drawMenu();
}
////////////////////////////////////////////////////////////////////DIODE METER
void executeDiode() {
// Displays Diode meter menu
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(2);
  //Draw lines and boxes
  tft.drawFastHLine(0, 87, SCREEN_HEIGHT, TFT_HIGHLIGHT);
  tft.drawRect(0, 68, SCREEN_HEIGHT, 98, TFT_HIGHLIGHT);
  //Draw text results
  tft.setCursor(2, 25);
  tft.print("DIODE METER");
  tft.setCursor(5, 50);
  tft.print("Ref: Cath.");
  tft.setCursor(10, 70);
  tft.print("CH");
  tft.write(0x23);
  tft.print("   mV");
  tft.setCursor(10, 90);
  tft.print("1: "); //CH1
  tft.setCursor(10, 110);
  tft.println("2: "); //CH2
  tft.setCursor(10, 130);
  tft.println("3: "); //CH3
  tft.setCursor(10, 150);
  tft.println("4: "); //CH3
  //Draw menu icons
  tft.drawFastHLine(0, 205, 135, TFT_symbol);
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  tft.setCursor(110, 210);
  tft.write(0x1B); //Check map in https://learn.adafruit.com/assets/103682  
  tft.setTextSize(2);
  tft.setTextColor(TFT_TEXT);
  //de-activate showMenu
  showMenu=false;

  digitalWrite(RL7,1);
  //CH1
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)]; //V
  }
  float voltageCH1 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN1)==0){
    tft.setCursor(35,90);
    tft.write(0xF2);
    tft.setCursor(50,90);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(35,90);
    tft.write(0xF1);
    tft.setCursor(50,90);
  }else{
    tft.setCursor(50,90);
  }
  tft.println(String(voltageCH1));
  
  //CH2
  digitalWrite(RL1, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)]; //V
  }
  float voltageCH2 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN1)==0){
    tft.setCursor(35,110);
    tft.write(0xF2);
    tft.setCursor(50,110);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(35,110);
    tft.write(0xF1);
    tft.setCursor(50,110);
  }else{
    tft.setCursor(50,110);
  }
  tft.println(String(voltageCH2));

  //CH3
  digitalWrite(RL1, 0);
  digitalWrite(RL5, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)]; //V
  }
  float voltageCH3 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN1)==0){
    tft.setCursor(35,130);
    tft.write(0xF2);
    tft.setCursor(50,130);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(35,130);
    tft.write(0xF1);
    tft.setCursor(50,130);
  }else{
    tft.setCursor(50,130);
  }
  tft.println(String(voltageCH3));

    //CH4
  digitalWrite(RL2, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN1[analogRead(AIN1)]; //V
  }
  float voltageCH4 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN1)==0){
    tft.setCursor(35,150);
    tft.write(0xF2);
    tft.setCursor(50,150);
  }else if(analogRead(AIN1)==4095){
    tft.setCursor(35,150);
    tft.write(0xF1);
    tft.setCursor(50,150);
  }else{
    tft.setCursor(50,150);
  }
  tft.println(String(voltageCH4));

  //Process the results
  int result = 0;
  if(inRange(voltageCH1,dio_low_limit,dio_high_limit) && inRange(voltageCH2,dio_low_limit,dio_high_limit) && inRange(voltageCH3,dio_low_limit,dio_high_limit) && inRange(voltageCH4,dio_low_limit,dio_high_limit)){
    result = 1; //1 passed, 0 failed.
  }else{
    result= 0;
  }
  //Draw results circle
  if(result==1){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_passed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_passed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("PASS");
  }else if(result==0){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_failed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_failed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("FAIL");
  }
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  digitalWrite(RL2, 0);
  delay(100);
  digitalWrite(RL5, 0);
  digitalWrite(RL7, 0);

  while(digitalRead(button_right)==1); //Waits while right button is not pressed
  delay(500); //If right button is pressed waits for 500ms (debouncing)

  //Restore variables
  showMenu=true;
  enterOption=false;
  tft.fillScreen(TFT_BACKGROUND);
  drawMenu();
}
/////////////////////////////////////////////////////////////////////VOLTMETER
void executeVoltage() {
// Displays voltmeter menu
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(2);
  //Draw lines and boxes
  tft.drawFastHLine(0, 87, SCREEN_HEIGHT, TFT_HIGHLIGHT);
  tft.drawRect(0, 68, SCREEN_HEIGHT, 98, TFT_HIGHLIGHT);
  //Draw text results
  tft.setCursor(15, 25);
  tft.print("VOLTMETER");
  tft.setCursor(5, 50);
  tft.print("Ref: Cath.");
  tft.setCursor(10, 70);
  tft.print("CH");
  tft.write(0x23);
  tft.print("   mV");
  tft.setCursor(10, 90);
  tft.print("1: "); //CH1
  tft.setCursor(10, 110);
  tft.println("2: "); //CH2
  tft.setCursor(10, 130);
  tft.println("3: "); //CH3
  tft.setCursor(10, 150);
  tft.println("4: "); //CH3
  //Draw menu icons
  tft.drawFastHLine(0, 205, 135, TFT_symbol);
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  tft.setCursor(110, 210);
  tft.write(0x1B); //Check map in https://learn.adafruit.com/assets/103682  
  tft.setTextSize(2);
  tft.setTextColor(TFT_TEXT);
  //De-activate showMenu
  showMenu=false;

  //CH1
  delay(100);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN2[analogRead(AIN2)]; //V
  }
  float voltageCH1 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN2)==0){
    tft.setCursor(35,90);
    tft.write(0xF2);
    tft.setCursor(50,90);
  }else if(analogRead(AIN2)==4095){
    tft.setCursor(35,90);
    tft.write(0xF1);
    tft.setCursor(50,90);
  }else{
    tft.setCursor(50,90);
  }
  tft.println(String(voltageCH1));
  
  //CH2
  digitalWrite(RL1, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN2[analogRead(AIN2)]; //V
  }
  float voltageCH2 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN2)==0){
    tft.setCursor(35,110);
    tft.write(0xF2);
    tft.setCursor(50,110);
  }else if(analogRead(AIN2)==4095){
    tft.setCursor(35,110);
    tft.write(0xF1);
    tft.setCursor(50,110);
  }else{
    tft.setCursor(50,110);
  }
  tft.println(String(voltageCH2));

  //CH3
  digitalWrite(RL1, 0);
  digitalWrite(RL5, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN2[analogRead(AIN2)]; //V
  }
  float voltageCH3 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN2)==0){
    tft.setCursor(35,130);
    tft.write(0xF2);
    tft.setCursor(50,130);
  }else if(analogRead(AIN2)==4095){
    tft.setCursor(35,130);
    tft.write(0xF1);
    tft.setCursor(50,130);
  }else{
    tft.setCursor(50,130);
  }
  tft.println(String(voltageCH3));

  //CH4
  digitalWrite(RL2, 1);
  delay(200);
  sum=0;
  for(int i=0; i<1000; i++){
    sum = sum+LUT_AIN2[analogRead(AIN2)]; //V
  }
  float voltageCH4 = sum; //mV Voltage calculation using LUT (1000/1000=1)
  if(analogRead(AIN2)==0){
    tft.setCursor(35,150);
    tft.write(0xF2);
    tft.setCursor(50,150);
  }else if(analogRead(AIN2)==4095){
    tft.setCursor(35,150);
    tft.write(0xF1);
    tft.setCursor(50,150);
  }else{
    tft.setCursor(50,150);
  }
  tft.setCursor(50,150);
  tft.println(String(voltageCH4));

  //Process the results
  int result = 0;
  if(inRange(voltageCH1,vol_low_limit,vol_high_limit) && inRange(voltageCH2,vol_low_limit,vol_high_limit) && inRange(voltageCH3,vol_low_limit,vol_high_limit) && inRange(voltageCH4,vol_low_limit,vol_high_limit)){
    result = 1; //1 passed, 0 failed.
  }else{
    result= 0;
  }
  //Draw results circle
  if(result==1){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_passed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_passed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("PASS");
  }else if(result==0){
    tft.fillCircle(SCREEN_HEIGHT/4, 183, 15, TFT_failed);
    tft.setTextSize(3);
    tft.setTextColor(TFT_failed);
    tft.setCursor(SCREEN_HEIGHT/4+20, 175);
    tft.print("FAIL");
  }
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  digitalWrite(RL2, 0);
  delay(100);
  digitalWrite(RL5, 0);

  while(digitalRead(button_right)==1); //Waits while right button is not pressed
  delay(500); //If right button is pressed waits for 500ms (debouncing)

  //Restore variables
  showMenu=true;
  enterOption=false;
  tft.fillScreen(TFT_BACKGROUND);
  drawMenu();
}

/////////////////////
void executeInfo() {
// Displays info menu
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(2);
  //Draw text results
  tft.setCursor(42, 25);
  tft.print("INFO");
  tft.setCursor(5, 50);
  tft.print("Ver.: 1.3");
  tft.setCursor(5, 70);
  tft.print("Contact:");
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setTextSize(1);
  tft.setCursor(5, 90);
  tft.println("Felipe Ramirez");
  tft.setCursor(5, 110);
  tft.println("< luisf.ramirez");
  tft.print("   ");
  tft.write(0x40);
  tft.print("udea.edu.co >");
  
  tft.drawFastHLine(0, 205, 135, TFT_symbol);
  tft.setTextSize(4);
  tft.setTextColor(TFT_symbol);
  tft.setCursor(110, 210);
  tft.write(0x1B); //Check map in https://learn.adafruit.com/assets/103682  
  tft.setTextSize(2);
  tft.setTextColor(TFT_TEXT);
  //De-activate ShowMenu
  showMenu=false;

  while(digitalRead(button_right)==1); //Waits while right button is not pressed
  delay(500); //If right button is pressed waits for 500ms (debouncing)

  //Restore variables
  showMenu=true;
  enterOption=false;
  tft.fillScreen(TFT_BACKGROUND);
  drawMenu();
}

/////////Function to check if a value is insede an interval
bool inRange(float val, float minimum, float maximum)
{
  return ((val>= minimum) && (val <= maximum));
}
