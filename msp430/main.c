#include <msp430.h>
#include <stdint.h>

#define interrupt(x) void __attribute__((interrupt (x)))

#define UPBTN BIT0
#define DOWNBTN BIT1
#define MODIFIER BIT2
#define BUTTONS (UPBTN | DOWNBTN | MODIFIER)
#define RCK BIT7
#define SER BIT6
#define SCK BIT7
#define DIGIT3 BIT6
#define DIGIT2 BIT5
#define DIGIT1 BIT4
#define HEAT BIT3
#define MOTOR BIT2
                      //   --A--
		      //  |     |
#define _A ( 1 << 7 ) //  F     B
#define _B ( 1 << 5 ) //  |     |
#define _C ( 1 << 3 ) //   --G-- 
#define _D ( 1 << 1 ) //  |     |
#define _E ( 1 << 0 ) //  E     C
#define _F ( 1 << 6 ) //  |     |
#define _G ( 1 << 4 ) //   --D--  .DP
#define DP ( 1 << 2 ) // decimal point

static const uint8_t glyphPattern[ 13 ] = {
	_A | _B | _C | _D | _E | _F,      // 0
	_B | _C,                          // 1
	_A | _B | _D | _E | _G,           // 2
	_A | _B | _C | _D | _G,           // 3
	_B | _C | _F | _G,                // 4
	_A | _C | _D | _F | _G,           // 5
	_A | _C | _D | _E | _F | _G,      // 6
	_A | _B | _C,                     // 7
	_A | _B | _C | _D | _E | _F | _G, // 8
	_A | _B | _C | _D | _F | _G,      // 9
	0,                                // none
	_A | _D | _E | _F | _G,		  // E
	_A | _E | _F,	          	  // R
};

char LEDBuffer[3] = {0xA, 0xA, 0xA};
enum heat {OFF, ON};
enum heat heater = OFF; 
enum directions {FWD, REV};
enum directions motor = FWD;



void initButtons(void) {
  P1DIR &= ~BUTTONS;		//Set pins 4-6 as inputs
  P1REN |= BUTTONS;		//Enable internal pull-up
  P1OUT |= BUTTONS;		//Set pull-up high
  P1IES |= DOWNBTN | UPBTN;	//Look for falling edges
  P1IFG &= ~BUTTONS;	//Just in case
  P1IE |= DOWNBTN | UPBTN;	//Now enable
}

void initLEDs(void) {
  TACTL = TASSEL_1 | MC_1;	//Set TimerA to use auxiliary clock in UP mode
  TACCTL0 = CCIE;		//Enable the interrupt for TACCR0 match
  TACCR0 = 32;			//Set TACCR0 which also starts the timer
  P1DIR |= SER | SCK;		//Set SR pins as outputs
  P1SEL = 0;			//In case the chip doesn't default to I/O
  P1DIR |= RCK;			//Set SR pins as outputs
  P2SEL = 0;			//Might as well save the bits when P2 is unconnected
}

void initSD16(void) {
  SD16INCTL0 =		//SD16 input control register channel 0
    SD16INCH_0 |	//Enable input
    SD16GAIN1 |		//Preamp gain
    SD16INTDLY0;	//Interrupt delay
  SD16AE = SD16AE2;	//Enable analog input 3 and disable digital circuitry
  SD16CTL = 		//SD16 control register
    SD16SSEL0 |		//Clock source
    SD16DIV_3 |		//Clock divider
    SD16REFON |		//Outout reference voltage to pin 5
    SD16XDIV_3; 	//Extended clock divider
  SD16IV = SD16IV_SD16MEM0; //Interrupt mode
  SD16CCTL0 =		//SD16 channel 0 control register
    SD16SC |		//Start converting now
    SD16IE |		//Enable interrupt
    0x2000 |		//Input buffer mode
    SD16UNI |		//SD16 unipolar mode
    SD16OSR_1024;	//Oversampling rate
}

char LED = 0;

void dispNumber(uint8_t glyph, uint8_t digit) {

  int currentGlyphPattern = glyphPattern[glyph] | (LED * DP);
// First byte
  for (uint8_t i = 0; i <= 7; ++i) {
    P1OUT &= ~(SCK | SER | RCK);
    P1OUT |= ((currentGlyphPattern & 1) * SER);
    currentGlyphPattern = currentGlyphPattern >> 1;
    P1OUT |= SCK;	//Need 100 ns or so delay, i guess
  }
//Second byte
  for (uint8_t i = 0; i <= 1; ++i) {	//With any luck gcc will do something nice to these loops
    P1OUT &= ~(SCK | RCK | SER);	//Leave NC bits low
    P1OUT |= SCK;			//Tick
  }
  for (int8_t i = 0; i <= 2; ++i) {
    P1OUT &= ~(SCK | SER);
    P1OUT |= ((digit == i) * SER);	//Pick a digit
    P1OUT |= SCK;	//Need 100 ns or so delay, i guess
  }
  P1OUT &= ~(SCK | SER);
  P1OUT |= heater * SER;
  P1OUT |= SCK;	//Need 100 ns or so delay, i guess
  P1OUT &= ~(SCK | SER);
  P1OUT |= motor * SER;
  P1OUT |= SCK;	//Need 100 ns or so delay, i guess
  P1OUT &= ~(SCK | SER);
  P1OUT |= SCK;	//Need 100 ns or so delay, i guess
  //All done! push signals out
  P2OUT |= RCK;
}

/* The following function was stolen from 
 * http://mspgcc.sourceforge.net/faq/x27.html
 */
void bin2bcd16(register char bcd[3], register uint16_t bin)
{
    register int i;
    register uint16_t decimal_0;
    register uint16_t decimal_1;

    i = 16;
    decimal_0 =
    decimal_1 = 0;
    __asm__ __volatile__(
        "1: \n"
        " rla   %[bin] \n"
        " dadd  %[decimal_0],%[decimal_0] \n"
        " dadd  %[decimal_1],%[decimal_1] \n"
        " dec   %[i] \n"
        " jnz   1b \n"
        " mov.b %[decimal_1],0(%[bcd]) \n"
        " mov.b %[decimal_0],2(%[bcd]) \n"
        " swpb  %[decimal_0] \n"
        " mov.b %[decimal_0],1(%[bcd]) \n"
        : [bcd] "+r"(bcd), [decimal_0] "+r"(decimal_0), [decimal_1] "+r"(decimal_1)
        : [bin] "r"(bin), [i] "r"(i));
}

volatile uint16_t setpoint = 200; //default temp 200C
volatile int16_t speed = 5; //default speed fast

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	//Stop WDT
  initLEDs();			//Setup LEDs
  initButtons();		//and buttons
  initSD16();
  BCSCTL3 |= LFXT1S_2;		//Set ACLK to use internal VLO (12 kHz clock)
  WRITE_SR(GIE);		//Enable global interrupts
  while(1) {
  }
}

int getTemp(void) {
  
  return 0;
}

uint8_t displayState = 1;

interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void) {
  switch (~P1IN & MODIFIER) {
    case MODIFIER:
      bin2bcd16(LEDBuffer, speed);
    break;
    default:
//      bin2bcd16(LEDBuffer, setpoint); TODO: alternate between setpoint and temp
      bin2bcd16(LEDBuffer, getTemp());
  }
  switch (displayState) {
    case 1:
      dispNumber(LEDBuffer[1] & 0x0f,0); // nibble 4 or perhaps - when implemented
      ++displayState;
    break;
    case 2:
      dispNumber((LEDBuffer[2] & 0xf0) >> 4,1); // nibble 5
      ++displayState;
    break;
    case 3:
      dispNumber(LEDBuffer[2] & 0x0f,2); // nibble 6
      displayState = 1;
  }
}

interrupt(PORT1_VECTOR) PORT1_ISR(void) {
  switch (~P1IN & MODIFIER) {
    default:
      switch (P1IFG & BUTTONS) {
        case DOWNBTN:
          if (setpoint != 0) {
            --setpoint;
          }
        break;
        case UPBTN:
          if (setpoint != 255) {
            ++setpoint;
          }
      }
    break;
    case MODIFIER:
      switch (P1IFG & BUTTONS) {
        case DOWNBTN:
          if (speed != -5) {
            --speed;
          }
        break;
        case UPBTN:
          if (speed != 5) {
            ++speed;
          }
      }
  }
  P1IFG &= ~BUTTONS; //yes, clear all three button bits
}

interrupt(SD16_VECTOR) SD16_ISR(void) {
  SD16CCTL0 &= ~SD16IFG;
  // for testing purposes, i'll just write to setpoint variable so it's displayed all the time.
  // This interrupt will take over the thermostat function.  TODO: trigger the SD16 from a timer interrupt
  setpoint = SD16MEM0;
  LED ^= 1;
}
