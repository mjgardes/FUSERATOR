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

static const uint8_t thermoTable[1024] = {
479, 474, 383, 339, 312, 292, 277, 264, 254, 246, 238, 232, 226, 220, 216, 211, 207, 203, 200, 197, 194, 191, 188, 186, 183, 181, 179, 177, 175, 173, 171, 169, 168, 166, 165, 163, 162, 160, 159, 158, 156, 155, 154, 153, 152, 151, 149, 148, 147, 146, 145, 145, 144, 143, 142, 141, 140, 139, 139, 138, 137, 136, 135, 135, 134, 133, 133, 132, 131, 131, 130, 129, 129, 128, 128, 127, 126, 126, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 120, 119, 119, 118, 118, 117, 117, 116, 116, 116, 115, 115, 114, 114, 114, 113, 113, 112, 112, 112, 111, 111, 111, 110, 110, 109, 109, 109, 108, 108, 108, 107, 107, 107, 106, 106, 106, 105, 105, 105, 105, 104, 104, 104, 103, 103, 103, 102, 102, 102, 102, 101, 101, 101, 101, 100, 100, 100, 99, 99, 99, 99, 98, 98, 98, 98, 97, 97, 97, 97, 96, 96, 96, 96, 95, 95, 95, 95, 95, 94, 94, 94, 94, 93, 93, 93, 93, 93, 92, 92, 92, 92, 92, 91, 91, 91, 91, 91, 90, 90, 90, 90, 90, 89, 89, 89, 89, 89, 88, 88, 88, 88, 88, 88, 87, 87, 87, 87, 87, 86, 86, 86, 86, 86, 86, 85, 85, 85, 85, 85, 85, 84, 84, 84, 84, 84, 84, 83, 83, 83, 83, 83, 83, 82, 82, 82, 82, 82, 82, 82, 81, 81, 81, 81, 81, 81, 81, 80, 80, 80, 80, 80, 80, 80, 79, 79, 79, 79, 79, 79, 79, 78, 78, 78, 78, 78, 78, 78, 78, 77, 77, 77, 77, 77, 77, 77, 76, 76, 76, 76, 76, 76, 76, 76, 75, 75, 75, 75, 75, 75, 75, 75, 74, 74, 74, 74, 74, 74, 74, 74, 74, 73, 73, 73, 73, 73, 73, 73, 73, 73, 72, 72, 72, 72, 72, 72, 72, 72, 72, 71, 71, 71, 71, 71, 71, 71, 71, 71, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33
//, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7
};

char LEDBuffer[3] = {0xA, 0xA, 0xA};
enum heat {OFF, ON};
enum heat heater = 0; 
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
  P1DIR |= SER | SCK | BIT3;	//Set SR pins as outputs (and Vref maybe?)
  P1SEL = BIT3 | BIT4 | BIT5;	//Set for the analogs, not for the buttons or SRs
  P2DIR |= RCK;			//Set SR pins as outputs
  P2SEL = 0;			//Might as well save the bits when P2 is unconnected
}

void initSD16(void) {
  SD16INCTL0 =		//SD16 input control register channel 0
    SD16INCH_2|		//Select channel A2
    SD16GAIN0 |		//Preamp gain
    SD16INTDLY0;	//Interrupt delay
  SD16AE = SD16AE2;	//Enable analog input 3 and disable digital circuitry
  SD16CTL = 		//SD16 control register
    SD16XDIV_3 | 	//Extended clock divider
    SD16DIV_3 |		//Clock divider
    SD16LP |		//Loe power (reduced clock) mode
    SD16SSEL0 |		//Clock source
    SD16VMIDON |	//Vref buffer on
    SD16REFON ;		//Output reference voltage to pin 5
  SD16IV = SD16IV_SD16MEM0; //Interrupt mode
  SD16CCTL0 =		//SD16 channel 0 control register
    SD16SC |		//Start converting now
    SD16IE |		//Enable interrupt
    SD16UNI |		//SD16 unipolar mode
    SD16OSR_1024;	//Oversampling rate
}

char LED = 0;

void dispNumber(uint8_t glyph, uint8_t digit) {

  int currentGlyphPattern = glyphPattern[glyph] | (LED * DP);
// First byte
  P2OUT &= ~RCK;
  for (uint8_t i = 0; i <= 7; ++i) {
    P1OUT &= ~(SCK | SER);
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
volatile uint16_t temperature = 0;
volatile int16_t speed = 5; //default speed fast

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;	//Stop WDT
  initLEDs();			//Setup LEDs
  initButtons();		//and buttons
  initSD16();
  BCSCTL3 |= LFXT1S_2;		//Set ACLK to use internal VLO (12 kHz clock)
  WRITE_SR(GIE);		//Enable global interrupts
  while(1) {
  motor = FWD;
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
      bin2bcd16(LEDBuffer, LED ? setpoint : temperature);// TODO: alternate between setpoint and temp
//      bin2bcd16(LEDBuffer, getTemp());
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
  // for testing purposes, i'll just write to setpoint variable so it's displayed all the time.
  // This interrupt will take over the thermostat function.  TODO: trigger the SD16 from a timer interrupt
  temperature = thermoTable[(SD16MEM0 & 0xffc0) >> 6];
  LED ^= 1;
  heater = (temperature < setpoint) ? ON : OFF;
}

