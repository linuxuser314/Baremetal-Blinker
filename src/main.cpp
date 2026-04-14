//main.cpp
//

//This program is a multi-state blinker/fader/flickerer utilizing non-blocking code.

//Created by Creed Truman, 4/1/2026
//Thanks to Uri Shaked's tutorial for inspiration for non-blocking LED blinking. No code was reused.
//https://blog.wokwi.com/5-ways-to-blink-an-led-with-arduino/
//Additional credit goes to Google Gemini for pointing out bugs before they became a problem (namely datatype mismatches and potential overflows) and giving general recommendations (My C was a little rusty).
//Additional credit for explaining the functionality of the AVR architecture and bare-metal features for direct port manipulation and timer setup.

//custom library for IO functions, pin definitions, and timer setup.
#include "myiolib.h"

//Global constants for pin numbers
constexpr PinStruct LED_1 = PIN_5;
constexpr PinStruct LED_2 = PIN_6;
constexpr PinStruct BUTTON = PIN_13;

//This is the startup state of the FSM (primarily used for rapid prototyping of new routines)
constexpr uint8_t STARTING_ROUTINE = 0;

//This global variable keeps up with the start time of the time bomb blink
unsigned long int timeBombStart = 0;

//This is the FSM counter variable.
uint8_t currentRoutine = STARTING_ROUTINE;

//Math utility functions
static long int max(long int a, long int b){
  return a<b?a:b;
}

//Returns 1 on the rising edge of the button press; otherwise returns 0.
static bool checkForButtonPress(const PinStruct pin) {

  //Returns 1 when it detects the button change from on to off
  //The static keyword makes this variable state persistent across all function calls.
  static bool buttonIsPressed = false;

  bool buttonVal = !myDigitalRead(pin);

  //This checks if the button is pressed and it was not previously pressed, update buttonIsPressed and returns 1.
  if (buttonVal && !buttonIsPressed) {
    buttonIsPressed = true;
    return 1;
  }
  //If the button is not pressed but it was previously being pressed, reset the buttonIsPressed for the next press.
  else if (!buttonVal && buttonIsPressed == true) {
    buttonIsPressed = false;
  }

  //End the function and return 0 if the button was not being pressed.
  return 0;

}

//Blinks each LED on and off in an alternating pattern of 1s
static void blink1() {
  //Simple
  bool state = (myMillis() / 1024) % 2;
  
  if(state){
    myDigitalWrite(LED_1, ON);
    myDigitalWrite(LED_2, OFF);
  }
  else{
    myDigitalWrite(LED_1, OFF);
    myDigitalWrite(LED_2, ON);
  }
}

//Blinks LED_1 on and off every 1s, blinks LED on and off every 0.25s
static void blink2() {
  
  //Still cleaner, but not as good as I would like it to be.
  unsigned long time = myMillis();
  
  if((time / 1024) % 2){
    myDigitalWrite(LED_1, ON);
  }
  else{
    myDigitalWrite(LED_1, OFF);
  }
  if(!((time / 256) % 2)){
    myDigitalWrite(LED_2, ON);
  }
  else{
    myDigitalWrite(LED_2, OFF);
  }
}

//Fun crazy blink
static void blink3() {
  unsigned long time = myMillis();
  
  if((time / 32) % 2){
    myDigitalWrite(LED_1, ON);
  }
  else{
    myDigitalWrite(LED_1, OFF);
  }

  if((time / 512) % 2){
    myDigitalWrite(LED_2, ON);
  }
  else{
    myDigitalWrite(LED_2, OFF);
  }
}

//Fades each LED in and out over 1s
static void fade1() {

  uint8_t level;
  uint16_t currentMillis = myMillis() % 2048;
  if(currentMillis < 1024){
    level = currentMillis / 4;
  }
  else{
    level = 255 - (currentMillis - 1024) / 4;
  }

  myAnalogWrite(LED_1, level);
  myAnalogWrite(LED_2, 255 - level);
}

//This starts the LEDs alternating blinking at 1s, then rapidly speed up until they both light up fully and then go out, then restarts the FSM.
static void tickingTimeBombBlink(){
  //How far are we into the routine?
  unsigned long currentTime = myMillis()-timeBombStart;

  //During the first 5 seconds, it starts slow then speeds up.
  if(currentTime <= 5000){
    bool val = currentTime / max(10UL, (5000UL - currentTime) >> 2) & 1;
    if(val){
      myDigitalWrite(LED_1, ON);
      myDigitalWrite(LED_2, OFF);
    }
    else{
      myDigitalWrite(LED_1, OFF);
      myDigitalWrite(LED_2, ON);
    }
  }

  //Between 5 and 6 seconds, lights are on.
  else if(currentTime >= 5000 && currentTime < 6000){
    myDigitalWrite(LED_1, ON);
    myDigitalWrite(LED_2, ON);
  }
  //Between 6 and 8 seconds, the lights are off.
  else if(currentTime >= 6000 && currentTime < 8000){
    myDigitalWrite(LED_1, OFF);
    myDigitalWrite(LED_2, OFF);
  }
  //After 8 seconds (when everything else is done), increment currentRoutine.
  else{
    currentRoutine ++;
  }
}

//I am using the main function here for better control and portability. I am putting it at the bottom to avoid having to use function prototypes.
int main(void){

  //Starts the timers for PWM and myMillis()
  initTimer0PWM();
  initTimer2Millis();
  initTimer1Servo50Hz();

  //Initializing Pins:
  myPinMode(LED_1, OUT);
  myPinMode(LED_2, OUT);
  myPinMode(BUTTON, IN);
  myDigitalWrite(BUTTON, ON); //turn on pull-up resistor for button pin. (IntelliCode suggested this comment before I had even typed it! Currently this is just a shortcut, I'll add a function later).


  //an infinite loop that allows the contents to be executed indefinitely.
  while(true){

    //This is the FSM state selector.
    switch (currentRoutine) {
      case (0):
        blink1();//Simple alternating blink
        break;
      case (1):
        blink2();//Fun alarm blink
        break;
      case (2):
        blink3();//Crazy blink
        break;
      case (3):
        fade1();//Alternating fade-in fade-out.
        break;
      case(4):
        tickingTimeBombBlink();//Starts slow, speeds up, turns on, off, and resets.
        break;
      default:
        currentRoutine = 0;  //resets the FSM when it reaches the end or if something goes wrong.
        break;
    }
  if(checkForButtonPress(BUTTON)){
    currentRoutine ++;//increments FSM state on the rising edge of the clock.

    if(currentRoutine == 4){
      //Initiates the time bomb start so that it starts fresh every time
      timeBombStart = myMillis();
	  disablePWM(LED_1);
	  disablePWM(LED_2);
    }
	else if(currentRoutine == 3){
		enablePWM(LED_1);
		enablePWM(LED_2);
	}
    
  }
  }
}