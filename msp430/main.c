#include <msp430.h>

#define interrupt(x) void __attribute__((interrupt (x)))

#define BUTTON1 BIT4
#define BUTTON2 BIT3
#define BUTTON3 BIT2
#define BUTTONS (BIT2 + BIT3 + BIT4)
#define SRDATA BIT7
#define DEMUX (BIT6 + BIT7)
#define DIGIT3 0
#define DIGIT2 BIT6
#define DIGIT1 BIT7
#define HEAT BIT0
#define MOTOR BIT1
                      //   --A--
		      //  |     |
#define _A ( 1 << 4 ) //  F     B
#define _B ( 1 << 7 ) //  |     |
#define _C ( 1 << 1 ) //   --G-- 
#define _D ( 1 << 3 ) //  |     |
#define _E ( 1 << 6 ) //  E     C
#define _F ( 1 << 5 ) //  |     |
#define _G ( 1 << 0 ) //   --D--  .DP
#define DP ( 1 << 2 ) // decimal point

static const char glyphPattern[ 13 ] = {
	_A | _B | _C | _D | _E | _F,      // 0
	_B | _C,                          // 1
	_A | _B | _D | _E | _G,           // u
	_A | _B | _C | _D | _G,           // 3
	_B | _C | _F | _G,                // 4
	_A | _C | _D | _F | _G,           // 5
	_A | _C | _D | _E | _F | _G,      // 6
	_A | _B | _C,                     // 7
	_A | _B | _C | _D | _E | _F | _G, // 8
	_A | _B | _C | _D | _F | _G,      // 9
	0,                                // none
	_B | DP,			  // !
	_B | _C | _E | _F | _G,	          // H
};

char displayBuffer[3] = {0xA, 0xA, 0xA};

void initButtons(void) {
  P1DIR &= ~BUTTONS;		//Set pins 4-6 as inputs
  P1REN |= BUTTONS;		//Enable internal pull-up
  P1OUT |= BUTTONS;		//Set pull-up high
  P1IES |= (BUTTON2 + BUTTON3);	//Look for falling edges
  P1IFG &= ~(BUTTON2 + BUTTON3);	//Just in case
  P1IE |= BUTTON2 + BUTTON3;	//Now enable
}

void initLEDs(void) {
  TACTL = TASSEL_1 | MC_1;	//Set TimerA to use auxiliary clock in UP mode
  TACCTL0 = CCIE;		//Enable the interrupt for TACCR0 match
  TACCR0 = 60;			//Set TACCR0 which also starts the timer
  P1DIR |= SRDATA;		//Set pin 9 as output (for now)
  P2DIR |= DEMUX;		//Set pins 12-13 as output
  P1OUT |= SRDATA;		//Turn on outputs
  P2OUT |= DEMUX;
  P1SEL = 0;			//In case the chip doesn't default to I/O
  P2SEL = 0;
}
/*
void initRelays(void) {
  P1DIR |= HEAT + MOTOR;	//Set pins 2-3 as outputs
  P1OUT &= HEAT + MOTOR;	//Turn off relay coils
}

void initThermistor(void) {
}

*/
void dispNumber(char glyph, char digit) {
  				//Turn off global interrupts while writing to 164 (not necessary, you idiot. interrupts are already off.)
int currentGlyphPattern = glyphPattern[glyph];
  for (char i = 0; i <=7; ++i) {
    P2OUT |= DEMUX;		//Lower clock and turn off LEDs
    P1OUT &= ~SRDATA;		//Blank SR input
    P1OUT |= currentGlyphPattern & SRDATA;	//Luckily SRDATA is BIT7!
    currentGlyphPattern = currentGlyphPattern << 1;
    P2OUT &= (digit | ~DEMUX);	// Select character on decoder, which raises clock
  }
}

void blank() {
  P2OUT |= DEMUX;
}

void display(char* displayString) {
  for (char i = 0; i<3; ++i) {
    dispNumber(displayString[i], i);
  }
}

void bufBCD (unsigned char x) { //Don't return the value this time, just write directly to the display buffer
  displayBuffer[0] = (x / 100);
  displayBuffer[1] = ((x % 100) / 10);
  displayBuffer[2] = ((x % 10)); 
}

volatile unsigned char setpoint = 200;
volatile char speed = 0;
enum modes {SETPOINT, SPEED};
volatile enum modes displayMode = SETPOINT;

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	//Stop WDT
  initLEDs();			//Setup LEDs
  initButtons();		//and buttons
  BCSCTL3 |= LFXT1S_2;		//Set ACLK to use internal VLO (12 kHz clock)
  WRITE_SR(GIE);		//Enable global interrupts
  while(1) {
  }
}

char displayState = 1;

interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void) {
  switch (displayState) {
    case 1:
      switch (displayMode) {
        case SETPOINT:
          bufBCD(setpoint);  //"hsync"
	  break;
	case SPEED:
	  bufBCD(speed);
      }
      dispNumber(displayBuffer[0],DIGIT1);
      ++displayState;
    break;
    case 2:
      dispNumber(displayBuffer[1],DIGIT2);
      ++displayState;
    break;
    case 3:
      dispNumber(displayBuffer[2],DIGIT3);
      displayState = 1;
  }
}

interrupt(PORT1_VECTOR) PORT1_ISR(void) {
  switch (~P1IN & BUTTON1) {
    case 0:
      displayMode = SETPOINT;
      switch (P1IFG & BUTTONS) {
        case BUTTON2:
          if (setpoint != 0) {
            --setpoint;
          }
        break;
        case BUTTON3:
          if (setpoint != 255) {
            ++setpoint;
          }
      }
    break;
    case BUTTON1:
      displayMode = SPEED;
      switch (P1IFG & BUTTONS) {
        case BUTTON2:
          if (speed != -5) {
            --speed;
          }
        break;
        case BUTTON3:
          if (speed != 5) {
            ++speed;
          }
      }
  }
  P1IFG = 0;
}
