
//myiolib.h
//This library was created by Creed Truman. This is my first version in VSCode!
//This version was last updated on 4/13/2026.

//Credit to Google Gemini for helping me debug and optimize the code, for explaining AVR architecture, and helping me get started with VSCode.


//This library is designed to provide basic IO functions, pin definitions, and timer setup for AVR microcontrollers.
//It is one of my first attemps at writing a C library.
//It is not fully functional by any means.

//TODO:
//Fully define all pins
//Seperate the digital and analog write functions with enable/disable PWM functions to reduce overhead
//Clean up the code and comments
//Add servo control functions
//Add serial IO functions

//This is necessary for IO port definitions, interrupt handling, and uint8_t
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

//Necessary for setting up pins
struct PinStruct{
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  uint8_t bit;
  bool isPWM;
  uint8_t modeBit;
  volatile uint8_t* timer;
  volatile uint8_t* PWMPtr;
};

constexpr PinStruct PIN_13 = {&PORTB, &DDRB, &PINB, 5, false, 0, nullptr, nullptr};
constexpr PinStruct PIN_5 = {&PORTD, &DDRD, &PIND, 5, true, COM0B1, &TCCR0A, &OCR0B};
constexpr PinStruct PIN_6 = {&PORTD, &DDRD, &PIND, 6, true, COM0A1, &TCCR0A, &OCR0A};

constexpr uint8_t ON = 1;
constexpr uint8_t OFF = 0;
constexpr uint8_t IN = 0;
constexpr uint8_t OUT = 1;

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
inline void initMillis(void){
  //Inititate Timer2 for myMillis()
  TCCR2A = (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = 249;
  TIMSK2 = (1 << OCIE2A);

  sei();
}



inline void initTimer0PWM(void){
  //This sets pins 5 and 6 (D5 and D6) to OUTPUT and 13 (B5) to INPUT
  //Sets Pin 1 to connect to timer and any necessary WGM bits
  TCCR0A = (1 << WGM00) | (1 << WGM01);
  TCCR0B = (1 << CS01) | (1 << CS00);
}

inline bool myDigitalRead(const PinStruct target){
  return *target.pin & (1 << target.bit);
}

inline void myPinMode(const PinStruct target, bool mode){
  if(mode){
    *target.ddr |= (1 << target.bit);
  }
  else{
    *target.ddr &= ~(1 << target.bit);
  }

}

inline void myDigitalWrite(const PinStruct target, bool level){
  if(target.isPWM){
    *target.timer &= ~(1 << target.modeBit);
  }
  if(level){
    *target.port |= (1 << target.bit);
  }
  else{
    *target.port &= ~(1 << target.bit);
  }
}
inline void myAnalogWrite(const PinStruct target, uint8_t level){
  if(target.isPWM){
    *target.PWMPtr = level;
    *target.timer |= (1 << target.modeBit);
  }
}

