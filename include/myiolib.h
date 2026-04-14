
//myiolib.h

long int max(long int a, long int b){
  return a>b?a:b;
}
long int min(long int a, long int b){
  return a<b?a:b;
}
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

volatile unsigned long systemMillis = 0;

ISR(TIMER2_COMPA_vect){
  systemMillis ++;
}

inline void initHardware(void){
  //This sets pins 5 and 6 (D5 and D6) to OUTPUT and 13 (B5) to INPUT
  //Sets Pin 1 to connect to timer and any necessary WGM bits
  TCCR0A = (1 << WGM00) | (1 << WGM01);
  TCCR0B = (1 << CS01) | (1 << CS00);

  //Inititate Timer2 for myMillis()
  TCCR2A = (1 << WGM21);
  TCCR2B = (1 << CS22);
  OCR2A = 249;
  TIMSK2 = (1 << OCIE2A);

  sei();
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

inline unsigned long myMillis(void){
  unsigned long time;
  cli();
  time = systemMillis;
  sei();
  return time;
}
