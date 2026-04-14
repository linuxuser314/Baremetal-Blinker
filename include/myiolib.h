
//myiolib.h
//This library was created by Creed Truman. This is my first version in VSCode!
//This version was last updated on 4/14/2026.

//Credit to Google Gemini for helping me debug and optimize the code, for explaining AVR architecture, and helping me get started with VSCode.


//This library is designed to provide basic IO functions, pin definitions, and timer setup for AVR microcontrollers.
//It is one of my first attemps at writing a C library.
//It is not fully functional by any means.

//TODO:
//Add serial IO functions

//This is necessary for IO port definitions, interrupt handling, and uint8_t
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

//Necessary for setting up pins
struct PWMStruct{
  uint8_t modeBit;
  volatile uint8_t* timer;
  volatile uint8_t* PWMPtr;
};
struct PinStruct{
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  uint8_t bit;
  PWMStruct PWMData;
};


//Placeholder
constexpr PWMStruct NO_PWM = {0, nullptr, nullptr};

//This is where I will define the pin constants. I am using the PinStruct to store all necessary information about each pin in one place, which should make it easier to write generic functions for pin manipulation and PWM control.
//I do not have PWM set up for all pins just yet, so I am using the NO_PWM for those pins (3, 9, 10, 11).
constexpr PinStruct PIN_0 = {&PORTD, &DDRD, &PIND, 0, NO_PWM};
constexpr PinStruct PIN_1 = {&PORTD, &DDRD, &PIND, 1, NO_PWM};
constexpr PinStruct PIN_2 = {&PORTD, &DDRD, &PIND, 2, NO_PWM};
constexpr PinStruct PIN_3 = {&PORTD, &DDRD, &PIND, 3, NO_PWM};
constexpr PinStruct PIN_4 = {&PORTD, &DDRD, &PIND, 4, NO_PWM};
constexpr PinStruct PIN_5 = {&PORTD, &DDRD, &PIND, 5, {COM0B1, &TCCR0A, &OCR0B}};
constexpr PinStruct PIN_6 = {&PORTD, &DDRD, &PIND, 6, {COM0A1, &TCCR0A, &OCR0A}};
constexpr PinStruct PIN_7 = {&PORTD, &DDRD, &PIND, 7, NO_PWM};
constexpr PinStruct PIN_8 = {&PORTB, &DDRB, &PINB, 0, NO_PWM};
constexpr PinStruct PIN_9 = {&PORTB, &DDRB, &PINB, 1, NO_PWM};
constexpr PinStruct PIN_10 = {&PORTB, &DDRB, &PINB, 2, NO_PWM};
constexpr PinStruct PIN_11 = {&PORTB, &DDRB, &PINB, 3, NO_PWM};
constexpr PinStruct PIN_12 = {&PORTB, &DDRB, &PINB, 4, NO_PWM}; 
constexpr PinStruct PIN_13 = {&PORTB, &DDRB, &PINB, 5, NO_PWM};

//These are just constants for readability and convenience.
constexpr uint8_t ON = 1;
constexpr uint8_t OFF = 0;
constexpr uint8_t IN = 0;
constexpr uint8_t OUT = 1;

inline bool myDigitalRead(const PinStruct target){
  return *target.pin & (1 << target.bit);
}

inline void myPinMode(const PinStruct target, bool mode){
  if(mode)	*target.ddr |= (1 << target.bit);
  else	*target.ddr &= ~(1 << target.bit);
}

inline void myDigitalWrite(const PinStruct target, bool level){
  if(level)	*target.port |= (1 << target.bit);
  else  *target.port &= ~(1 << target.bit);
}
inline void myAnalogWrite(const PinStruct target, uint8_t level){
    *target.PWMData.PWMPtr = level;
}

inline void enablePWM(const PinStruct target){
    *target.PWMData.timer |= (1 << target.PWMData.modeBit);
}
inline void disablePWM(const PinStruct target){
    *target.PWMData.timer &= ~(1 << target.PWMData.modeBit);
}

//Primary robot drive function
inline void drive(int8_t left, int8_t right){
	//Drives the robot.
	//It starts by taking 3000 (1.5ms).
	//Then it takes the left and right variables (which range from -100 to 100) and multiplies them by 4.
	//This gives us a range from -400 to 400.
	//Adding them to the 3000 gives us a range from 2600 to 3400, or 1.3ms to 1.7ms to control the servos.
	//The right motor is reversed because of the way it is oriented on the robot.
	OCR1A = 3000 + (uint16_t)left * 4;
	OCR1B = 3000 - (uint16_t)right * 4;
}


//defines the functions necessary for setting up, running, and updating myMillis().
volatile unsigned long systemMillis = 0;
ISR(TIMER2_COMPA_vect){
  systemMillis ++;
}
inline unsigned long myMillis(void){
  unsigned long time;
  cli();
  time = systemMillis;
  sei();
  return time;
}
inline void initTimer2Millis(void){
  //Inititate Timer2 for myMillis()
  TCCR2A = (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = 249;
  TIMSK2 = (1 << OCIE2A);

  sei();
}


//this initiates Timer0 PWM for pins 5 and 6.
inline void initTimer0PWM(void){
	//Sets Pins 5 and 6 Waveform Generation Mode to Mode 3: Fast PWM
	//This means that Timer0 starts at 0, counts up to 255, and then resets to 0.
	TCCR0A = (1 << WGM00) | (1 << WGM01);
	//Sets the clock select bits to use a prescaler of 64, which gives us a PWM frequency of about 976Hz.
	TCCR0B = (1 << CS01) | (1 << CS00);
}
//This initiates Timer1 for 50Hz servo control.
inline void initTimer1Servo50Hz(void){

	myPinMode(PIN_9, OUT);
	myPinMode(PIN_10, OUT);

	//Setting the Waveform Generation Module to Mode 14: Fast PWM with ICR1 as TOP.
	//This allows us to set a custom TOP value for a specific PWM frequency, which is necessary for accurate servo control.
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12);

	//Sets the TOP value for 50Hz
	ICR1 = 39999;

	//Sets the prescaler to 8 so that it only increments the timer every 8 clock cycles
	//This, combined with the TOP value, gives us a PWM frequency of 50Hz, which is standard for servo control.
	TCCR1B |= (1 << CS11);

	//We need to set the Compare Output Mode (COM) bits to enable the multiplexer to select PWM instead of Digital.
	//This is equivalent to enablePWM
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1);

	//Set the motors to be stopped initially (1.5ms pulse)
	OCR1A = 3000;
	OCR1B = 3000; 

}





