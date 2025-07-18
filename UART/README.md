# UART

## uartheader.h
### Header File for LPC2129
```c
#include <lpc21xx.h>
```
### To get delay in milli seconds.
```c
void delay_ms(unsigned int); 
```
### LCD Section
```c
void lcd_init(void);	/*lcd initilisation*/
void lcd_cmd(unsigned char); /*lcd command input*/
void lcd_data(unsigned char); /*lcd data intput*/
void lcd_string(char *);	/*lcd string input*/
```
### UART Section
```c
void u_init(unsigned int); /*uart initilisation*/
/*1 character*/
void u_tx_char(unsigned char); /*Trasmit 1 character*/
unsigned char u_rx_char(void); /*Receive 1 character*/
/*String*/
void u_tx_string(char *); /*Transmit String*/
void u_rx_string(char *, unsigned int); /*Receive String*/
```





## uartdriver.c
```c
#include "uartheader.h"
```
Delay defination in milli seconds.

- Frequency selection
	- For `15 Mhz`, VPBDIV = `0`
	- For `60 Mhz`, VPBDIV = `1`
	- For `20 Mhz`, VPBDIV = `2` 
	- Reset to `default`, VPBDIV = `3`

```c
void delay_ms(unsigned int sec)
{
	int a[] = {15, 60, 30, 15};
```
Set the Program Counter to 0.
```c
	T0PC = 0; 
```
To set the Program Register according to the clock pulse in milli seconds	
```
	T0PR = a[VPBDIV]*1000 - 1; 
	T0TC = 0; /*Set the initial value for Timer Counter*/
	
	/*
	T0TCR : 32 bit register (only Bit 0  and Bit 1 is used)
	1 : To Start the Timer 
	0 : To Stop the Timer
	 */
	T0TCR = 1; /*Start the Timer*/
	
	/*
	 Monitor the T0TC till it hit the Max value (15 Mhz : 15000000 - 1)
	 whenever the program overflow the T0TC will increment 
	 */

	while(T0TC < sec);

	T0TCR = 0; /*Stop the Timer*/
}


/*To initilise the UART*/
/*
 1. Receive the Baud rate for communication
 2. Generate frequency based on CCLK

--------------------------------------------------------------------------------
U0LCR : Uart0 Line Control Register : 32 bit register | only 8 bit used
--------------------------------------------------------------------------------
|Bit 7|Bit 6    |Bit 5 |Bit 4  |Bit 3        |Bit 2         |Bit 1  |  Bit 0   | 
|DLAB |Set Break|Parity Select |Parity Enable|No of Stop Bit|Word Length Select|
--------------------------------------------------------------------------------

( Bit 0 | Bit 1 ) : Size of frame
  0	  0	  : 5 bit frame
  1	  0 	  : 6 bit frame
  0	  1	  : 7 bit frame
  1	  1	  : 8 bit frame	  

( Bit 2 ) : Number of Stop Bit 
  0	  : 1 Stop Bit
  1 	  : 2 Stop Bit

	if baud rate <= 57600 bps | 1 Stop bit
      	if baud rate > 	57600 bps | 2 Stop bit

---------------------------------------------------------------
Parity : Used to check error Detection method between 2 devices
---------------------------------------------------------------
( Bit 3 ) : Parity Enable
  0	  : disable parity
  1	  : enable parity

( Bit 4 | Bit 5 ) : Parity Select
  0	  0	  : 0 | odd parity	
  1	  0	  : 1 | even parity
  0	  1	  : Forced '1' stick parity
  1	  1	  : Forced '0' stick parity
			
  *Note : Stick Parity is used to implement protocole such as UDS, CAN.

( Bit 6 ) : Break Control
  0	  : disable break transmission
  1	  : enable break transmission

  + break control not used in UART communication
  + used for hardware debugging tools

( Bit 7 ) : DLAB | Divisor Latch Access Bit
  0 	  : baud rate locked | Tx/Rx of data is possible in UART
  1	  : baud rate unlocked	| Tx/Rx of data is not possible in UART

-----------------------------------------------------------------------

	+ Unlock baudrate using DLAB : U0LCR = 0x83;
			|7|6|5|4|3|2|1|0|
			|1|0|0|0|0|0|1|1|
		- 8 bit word length
		- 1 stop bit
		- parity disable
		- unlock baudrate
	+ Lock baudrate using DLAB : U0LCR = 0x03;
			|7|6|5|4|3|2|1|0|
			|1|0|0|0|0|0|1|1|
		- 8 bit word length
		- 1 stop bit
		- parity disable
		- lock baudrate
-----------------------------------------------------------------------
 3. Get Baud rate setting value 
 	clock_value  = PCLK / (16 x baudrate)
	Baud rate setting register
		U0DLL : 8 bit register | Uart0 Divisor Latch LSB
			+ used to store Lower byte's
			+ store : 0-255

		U0DLM : 8 bit register | Uart0 Divisor Latch MSB	
			+ used to store Higher byte's
			+ store : value greater than 255

	U0DLL = clock_value & 0xFF;
	U0DLM = clock_value >> 8 & 0xFF;
-----------------------------------------------------------------------
U0LSR : Uart0 Line Status Register : 32 Bit register | only 8 bit used 
-----------------------------------------------------------------------
	|Rx FIFO| TEMT | THRE | BI | FE | PE | OE | RDR |

	7 Rx FIFO : Receiver First In First Out
		0 : No data in FIFO | No overrun
		1 : 1 byte is availble in Rx FIFO | Rx FIFO was full, and new data arrived (old data lost).
	6 TEMT    : Transmitter Empty
		0 : U0THR/U0TSR contain data
		1 : U0THR/U0TSR doesn't contain data
	5 THRE    : Transmitter Hold Register Empty
		0 : U0THR doesn't contain data
		1 : U0THR contain data
	4 BI      : Break Interrupt
		0 : No break condition
		1 : Break condition detected
	3 FE	  : Framing Error
		0 : No framing error (valid stop bit detected)
		1 : Framing error detected (stop bit was not as expected)
	2 PE	  : Parity Error
		0 : Parity error status : inactive
		1 : Parity error status : active
	1 OE	  : Over Run Error | Data Lost
		0 : Overrun error status : inactive
		1 : Overrun error status : active
	0 RDR	  : Recieve Data Ready
		0 : U0RBR is empty/no data
		1 : U0RBR contain valid data

-------------------------------------------------------------------------------
 4. Enable TxD & RxD pin using PINSEL Register
 	PINSEL : used to select GPIO Pin | 32 bit register
	
 31 30 29 28 27	26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9  8  7  6  5  4  3  2  1  0
|0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |0 |

		2 bits is equal to 1 pin
			

		+ PINSEL0 : P0.0  - P0.15 
		+ PINSEL1 : P0.16 - P0.31
		+ PINSEL2 : P1.16 - P1.31
		
		Bit 0   Bit 1   : TxD Pin configuration
		0	0 	: default GPIO pin
		1	0	: TxD pin (1st alternative pin)
		0	1	: PWM1 pin (2nd alternative pin)
		1	1	: Reserved

	PINSEL0 = 0x0; : Default GPIO Pin
	PINSEL0 = 0x1; : TxD Pin
	PINSEL0 = 0x2; : PWM1 Pin
	PINSEL0 = 0x3; : Reserved

		Bit 2   Bit 3   : RxD Pin configuration
		0	0 	: default GPIO pin
		1	0	: RxD pin (1st alternative pin)
		0	1	: PWM3 pin (2nd alternative pin)
		1	1	: EINT0 (3rd alternative pin)

	PINSEL0 = 0x0; : Default GPIO Pin
	PINSEL0 = 0x4; : RxD Pin
	PINSEL0 = 0x8; : PWM3 Pin
	PINSEL0 = 0xC; : EINT Pin


	PINSEL0 = 0x5; |0|1|0|1| : P0.0 (TxD) and P0.1 (RxD)
----------------------------------------------------------------------------
U0THR : Uart0 Transmit Hold Register | 8 bit register | write only register
----------------------------------------------------------------------------
 U0THR  : Transmit Hold Register  	 : 1 byte | default value : 0
	+ Store data waiting to be transmitted (software write here)
----------------------------------------------------------------------------
 U0TSR  : Transmit Shift Register 	 : 1 byte | default value : 0
	+ Actively transmit data bit by bit (hardware controlled)
----------------------------------------------------------------------------
 		THRE (Bit 5 of LSR)
 THRE   : Tranmit Holding Register Empty : 1 bit  | default value : 1
	+ Flag = 1 when U0THR is empty (ready for new data)
----------------------------------------------------------------------------

Working of these 3 register
	- CPU write to U0THR -> Data move to U0TSR (if idle)
	- U0TSR shifts out bits serially (start -> data -> stop)
		+ create a frame and send data bit by bit
			Start bit (0)
			Data bits (LSB first , 1 0 0 0 0 0 1 0 for 'A')
			Parity Bit (if enabled) : optional
			Stop bit (1)
		+ THRE = 1 (since U0THR is now empty and can accept new data)
	- THRE flag indicates when U0THR is empty (ready for new data)
		+ THRE = 1 -> U0THR is empty (cpu  can write new data)
		+ THRE = 0 -> U0THR is full (still holding data wait before writing)
	- FIFO mode allows buffering multiple bytes before transmission

Usage : (Polling Method)
	U0THR = 'A';
	while(THRE == 0); // Waiting for completly transmitting data

-----------------------------------------------------------------------------
U0RBR : Uart0 Receiver Buffer Register : 8 bit register : read only
-----------------------------------------------------------------------------
 U0RBR : Receive Buffer Register : 1 byte | default value : 0 
       + Holds complete received byte for CPU to read	 	
-----------------------------------------------------------------------------
 U0RSR : Receive Shift Register  : 1 byte | default value : 0
       + Hardware register that collects incoming bits (hardware controlled)
-----------------------------------------------------------------------------
 		RDR (Bit 0 of LSR)
 RDR    : Receive Data Ready : 1 bit  | default value : 1
	+ Flag = 1 when U0RBR contain new data (Read via LSR)
-----------------------------------------------------------------------------
Usage : #define RDR ( U0LSR & 1 )
	unsigned char uart0_rx (void )
	{
		while( RDR == 0);
		return U0RBR;
	}
-----------------------------------------------------------------------------
Problem 			:	Likely Cause
Data never ready (RDR = 0)	:	No incoming data
Garbage value			:	Baud rate mismatch
Overrun Error			:	CPU too slow to read
Framing Error			: 	Stop bit issue
-----------------------------------------------------------------------------
			     
*/

void u_init(unsigned int baudrate){
	unsigned int pclk = 0, clock_value;
	
	int a[] = {15, 60, 30, 15}; /*Frequency Selection Array*/
	pclk = a[VPBDIV]*1000000; /*Generate Frequency Basec clock*/
	clock_value = pclk/(16 * baudrate); /*generate the clock value for DLL and DLM*/
	
	PINSEL0 |=5;	/*Enable TxD and RxD*/
	U0LCR = 0X83;	/*Unlocking DLAB*/
	U0DLL = 0xFF & clock_value; /*Setting LSB of Baudrate*/
	U0DLM = (clock_value>>8) & 0xFF; /*Setting MSB of Baudrate*/
	U0LCR = 0x03; /*Disable DLAB*/
}	

/*Character Section*/
/*Tx Character in UART*/

#define U0THRE ( U0LSR >> 5 & 1 )

void u_tx_char(unsigned char data)
{
	U0THR = data; /*Copy all the 8 Bit to U0THR register*/
	/*
	 If U0THRE is 0 U0THR has data
	 	(when U0THR has data, data is immediatly moved to  U0TSR and it transfer it bit by bit, when all the data is copied to U0TSR , U0THRE will become 1 )
	 If U0THRE is 1 U0THR is empty and waiting for data
	 */
	while( U0THRE == 0); /*Waiting till all the bits transmit bit by bit*/
}

/*Rx Character in UART*/

#define U0RDR  (U0LSR & 1)

unsigned char u_rx_char(void)
{
	while(U0RDR == 0); /*U0RSR is Not holding any data*/
	/*Data is shifted to U0RBR after data is completly received by U0RSR*/
	return U0RBR; /*Sending data to the receiver function*/
}


/*String Section*/
/*Tx String in UART*/
void u_tx_string(char *p)
{
	while(*p)
	{
		while(U0THRE == 0); /*Waiting till all the bits are trasmitted bit by bit*/
		U0THR = *p++; /*Loading Character from string 1 by 1*/
	}

}



/*Rx String in UART*/
/*Take a pointer which will hold a string and max size of the array*/
void u_rx_string(char *a, unsigned int size)
{
	int i;

	for(i = 0; i<size; i++)
	{
		while(U0RDR == 0); /*U0RSR is not holding data , waiting for data*/
		a[i]= U0RBR; /*Storing into a array to create a string*/
		
		/*Breaking condition*/
		if(a[i] == '\r')	/*When enter key is pressed*/
			break;
	}

	/*Add NULL at the end in place of '\r' */
	a[i] = '\0';
}
```

