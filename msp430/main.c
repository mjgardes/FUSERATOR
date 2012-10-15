#include <msp430.h>

#define interrupt(x) void __attribute__((interrupt (x)))

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
  P1DIR &= ~BUTTONS;		//Set pins 4-6 as inputs (check this)
  P1REN |= BUTTONS;		//Enable internal pull-up
  P1OUT |= BUTTONS;		//Set pull-up high
}

void initLEDs(void) {
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
    P1OUT &= ~SRDATA;	//Blank SR input
//    P1OUT |= ((glyphPattern[glyph] &  (1 << i)) >> i) << 7;	//Read SR bit from table and write to SR data pin -- The compiler does not do this well
    P1OUT |= currentGlyphPattern & SRDATA;	//I need to stop swapping endianness of this step  //Luckily SRDATA is BIT7!
    currentGlyphPattern = currentGlyphPattern << 1;
    P2OUT &= (digit | ~DEMUX); // Select character on decoder, which raises clock
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

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	//Stop WDT
  initLEDs();			//Setup LEDs

  BCSCTL3 |= LFXT1S_2;		//Set ACLK to use internal VLO (12 kHz clock)
  TACTL = TASSEL_1 | MC_1;	//Set TimerA to use auxiliary clock in UP mode
  TACCTL0 = CCIE;		//Enable the interrupt for TACCR0 match
  TACCR0 = 100;			//Set TACCR0 which also starts the timer
  WRITE_SR(GIE);		//Enable global interrupts
  displayBuffer = {1, 2, 3};
  while(1) {
  }
}

int testDigit = 1;

interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void) {
  dispNumber(displayBuffer[0],DIGIT1);	// Display H (0xC) on DIGIT1
  __delay_cycles(2400);	// Just pause here for testing
  dispNumber(displayBuffer[1],DIGIT2);	// Display 1 (0x1) on DIGIT2
  __delay_cycles(2400);	// Just pause here for testing
  dispNumber(displayBufffer[2],DIGIT3);	// Display ! (0xB) on DIGIT3
  __delay_cycles(2400);	// Just pause here for testing
  blank();			// Give all the digits equal time
}

interrupt(PORT1_VECTOR) TIMERA0_ISR(void) {
  
}
