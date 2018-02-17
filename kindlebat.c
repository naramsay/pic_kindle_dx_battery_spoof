/*
 * File:   kindlebat.c
 * Author: neil
 *
 * Created on March 25, 2016, 8:50 PM
 */

/*  Example of I2C reading 2 bytes (rr) using BusPirate 
Write address 
I2C>[0xaa 0x00 [0xab rr]
I2C START BIT
WRITE: 0xAA ACK
WRITE: 0x00 ACK
I2C START BIT
WRITE: 0xAB ACK
READ: 0xAA
READ:  ACK 0xEE
NACK
I2C STOP BIT

Example of writing using BusPirate
I2C>[0xaa 0x00 0x10]
I2C START BIT
WRITE: 0xAA ACK
WRITE: 0x00 ACK
WRITE: 0x10 ACK
I2C STOP BIT
*/

// CONFIG1
#pragma config FOSC = INTOSC    // INTOSC oscillator: I/O function on CLKIN pin
//#pragma config WDTE =  ON       // Watchdog Timer Enable (WDT enabled)
#pragma config WDTE =  ON       // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR/VPP pin function is MCLR
#pragma config CP = ON          // Flash Program Memory Code Protection enabled
#pragma config CPD = ON         // Data memory code protection is enabled
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config CLKOUTEN = OFF   // CLKOUT function is disabled.
                                // I/O or oscillator function on the CLKOUT pin
#pragma config IESO = ON        // Internal/External Switch-over mode is enabled
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor is enabled

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection off
//#pragma config VCAPEN = OFF     // All VCAP pin functionality is disabled
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow will cause a Reset
#pragma config BORV = LO        // Brown-out Reset Voltage (Vbor),
                                // low trip point selected.
#pragma config LVP = ON         // Low-voltage programming enabled

#include <xc.h>
#include <pic.h>
#include <pic16f1824.h> 
#include "uart.h"

#define I2C_address   0xAA    /* I2C address of the slave node */ 
#define RX_ELMNTS   128	


// array for master to read from
volatile unsigned char I2C_Array[RX_ELMNTS] =
{0x00,0x40,0xb6,0x17,0x26,0x00,0xa1,0x04,0x75,0x0f,0x04,0x63,0xb7,0x0f,
0xd8,0x0f,0xb7,0x0f,0xd8,0x0f,0x67,0x00,0x27,0x09,0xff,0xff,0x67,0x00,0x27,
0x09,0x00,0x00,0x35,0x0e,0x0f,0x00,0x0f,0x00,0x82,0x08,0x02,0x00,0x0c,0x00,
0x63,0xe0,0xa0,0x07,0x21,0x00,0xf8,0x9b,0xc3,0x1b,0xff,0xff,0xa8,0xf6,0xff,0x00,
0xe3,0x03,0x4b,0x4e,0x00,0x00,0x05,0x00,0x00,0x00,0x21,0xa9,0xc7,0x48,0x38,0x85,
0x7f,0x66,0x6c,0x46,0xdf,0x20,0x36,0x09,0x00,0x01,0x52,0xa4,0xcf,0x0f,0x3f,0x24,
0x00,0x00,0x71,0xff,0x91,0xff,0x62,0xff,0x8d,0x04,0xd9,0x05,0x00,0x00,0x00,0x01,
0x00,0x07,0x00,0x00,0x2d,0x78,0x37,0x6d,0x7a,0xbb,0x21,0xa9,0xc7,0x48,0x38,0x85,
0x7f,0x66,0x00,0xa9};




unsigned char dbuf = 0;     // index pointer
unsigned char junk = 0;          // used to place unnecessary data
unsigned char clear = 0x00;   
unsigned int first = 0;


void initialize(void);

void main(void) {
//    UART_Init(9600);
    initialize();
    while(1)
   {
      asm("CLRWDT");          // clear WDT
   }
}


void initialize(void)
{
    //char text[]="init\n";
//uC SET UP
    OSCCON = 0b01111010; 		// Internal OSC @ 16MHZ
    //OSCCON = 0b01101010; 		// Internal OSC @ 4MHZ
    OPTION_REG = 0b11010111;    // WPU disabled, INT on rising edge, FOSC/4
                                // Prescaler assigned to TMR0, rate @ 1:256
    WDTCON = 0b00010111;        // Prescaler 1:65536
                                // period is 2 sec (RESET value)
    PORTC = 0x00;               // Clear PORTC
    LATC = 0x00;                // Clear PORTC latches
    ANSELC = 0x00;		// OMG!
    TRISC = 0b00011011;         // Set RC0(SCL), RC1(SDA) as inputs for I2C
//I2C SLAVE MODULE SET UP 
    SSP1STAT = 0b11000000;       // Slew rate control disabled for standard
                                // speed mode (100 kHz and 1 MHz)
    				// SMbus input logic
    SSP1CON1 = 0b00110110; 		// Enable serial port, I2C slave mode, 
                                // 7-bit address
    SSP1CON2bits.SEN = 0;        // Clock stretching is enabled
    //SSP1CON2bits.GCEN = 1;	// General Call Enable
    SSP1CON3bits.BOEN = 1;       // SSPBUF is updated and NACK is generated
                                // for a received address/data byte,
                                // ignoring the state of the SSPOV bit
                                // only if the BF bit = 0
    SSP1CON3bits.SDAHT = 1;		// Minimum of 300 ns hold time on SDA after
                                // the falling edge of SCL
    SSP1CON3bits.SBCDE = 0;		// Enable slave bus collision detect interrupts
    SSP1ADD = I2C_address; // Load the slave address
    SSP1IF = 0;                  // Clear the serial port interrupt flag
    BCL1IF = 0;                  // Clear the bus collision interrupt flag
    BCL1IE = 1;                  // Enable bus collision interrupts
    SSP1IE = 1;                  // Enable serial port interrupts
    PEIE = 1;                   // Enable peripheral interrupts
    GIE = 1; 					// Enable global interrupts
    //UART_Write_Text(text);
//    UART_Write(1);
}//end initialize


/*************************** ISR ROUTINE **************************************/
/*************************** ISR ROUTINE **************************************/
void interrupt ISR(void)
{
    if(SSP1IF)                               // check to see if SSP interrupt
    {
        if(!SSP1STATbits.R_nW)			// master write (R_nW = 0)
        {
	// STATE 1
            if(!SSP1STATbits.D_nA)        // last byte was an address (D_nA = 0)
            {
                junk = SSP1BUF;			// read buffer to clear BF
		SSP1CON1bits.CKP = 1;            // release CLK
            }
            
	// STATE 2    
	    if(SSP1STATbits.D_nA)                // last byte was data (D_nA = 1)
            {
	       if (SSP1STATbits.BF){
               // first time around data is an index pointer to array
	       if (first == 0){			
	          dbuf = SSP1BUF;
	          first++;
	       }
	       // second time around data is data - so write to array using previous index
	       else {				
                  I2C_Array[dbuf] = SSP1BUF;
		  first = 0;
	       }
	   }
		   if(SSP1CON1bits.WCOL)           // Did a write collision occur?
		   {
                       SSP1CON1bits.WCOL = 0;       // clear WCOL bit
                       junk = SSP1BUF;              // clear SSPBUF
		   }
        	   SSP1CON1bits.CKP = 1;    		// release CLK
              }
	   // STATE 5
	   else {
                  SSP1CON1bits.CKP = 1;
		  dbuf = 0;
	          first = 0;
                }
           }
    	
        // STATE 3	
        if(SSP1STATbits.R_nW)                // Master read (R_nW = 1)
        {
            if(!SSP1STATbits.D_nA)         // last byte was an address (D_nA = 0)
            {
                junk = SSP1BUF;                  // dummy read to clear BF bit
                SSP1BUF = I2C_Array[dbuf];   // load SSPBUF with data
		first = 0;
                SSP1CON1bits.CKP = 1;        // release CLK
            }

	// STATE 4
            if(SSP1STATbits.D_nA)       // last byte was data
            {
		if (dbuf++ < RX_ELMNTS)
                {               // Does data exceed number of allocated bytes?
                    SSP1BUF = I2C_Array[dbuf++];   // load SSPBUF with data
                }                                       // and increment index
                else
                {
                    junk = SSP1BUF;      // dummy read to clear BF bit
                }
	        first = 0;
                SSP1CON1bits.CKP = 1;    // release CLK
            }
        }
    }

    if(BCL1IF)                       // Did a bus collision occur?
    {
        junk = SSP1BUF;              // clear SSPBUF
		BCL1IF = 0;                  // clear BCL1IF
		SSP1CON1bits.CKP = 1;        // Release CLK
    }
    SSP1IF = 0;                      // clear SSP1IF

}