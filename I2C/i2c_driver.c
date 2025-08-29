/*I2C Driver*/

/*
I2C : Inter Integrated Circuit

Two Wire

SDA : Serial Data Line.
SCL : Serial Clock Line.

Data Frame in I2C

Byte write frame : write 1 byte of data from master to slave.
Byte read frame : read 1 byte of data from slave to master.
Sequential read frame : used to read multiple byte of data from slave to master.

I2C Bus Frame and State

START  
STOP 
ACK 
NACK 

START → Slave Address + Read/Write bit → ACK → Memory Address → ACK → Data → ACK → STOP


I2C Registers

I2CONSET : I2C Control Set Register.

I2CONCLR : I2C Control Clear Register.

I2STAT : I2C Status Register.

I2DAT : I2C Data Register.

I2SCLH/I2SCLL : I2C Serial Clock High/Low.


 */


/*DS1307 RTC*/

/*
 64 byte onchip memory
 	- 8 Mapped time keeping registers.
	- 56 General Purpose registers.
	

To ensure the correct time and date, an external crystal oscillator with a value of 
32.768 kHz must be connected to the RTC.

The address mapping for the DS1307 is as follows:

0x00: Seconds (0-59) 
0x01: Minutes (0-59) 
0x02: Hours (1-12 or 0-23) 
0x03: Day (1-7) 
0x04: Date (1-31) 
0x05: Month (1-12) 
0x06: Year (0-99) 
0x07: Control Register 
 
 */
// header.h
#define CSO (1<<7)
extern void spi0_init(void);
typedef unsigned short int ushort;
typedef unsigned char us;
us spi0(us data);
ushort read_mcp3204(us ch_num);

// mcp3204-driver.c
#include <lpc21xx.h>
#include "header.h"

ushort read_mcp3204(us ch_num) {
    us byteL = 0, byteH = 0;
    ushort result = 0;
    ch_num <<= 6; // set the ch-num

    IOCLR0 = CSO; // select slave
    spi0(0x06); // start bit, SGL mode, D2
    byteH = spi0(ch_num); // CH select
    byteL = spi0(0x00);
    IOSET0 = CSO; // deselect slave

    byteH &= 0x0F; // mask higher nibble
    result = (byteH << 8) | byteL; // merge result
    return result;
}

// main_mcp3204.C
#include "header.h"

main() {
    ushort temp;
    uarto_init(9600);
    spi0_init();
    uarto_tx_string("MCP3204 Testing \r\n");

    while(1) {
        temp = read_mcp3204(0); // read CH0
        uarto_tx_integer(temp);
        uarto_tx_string("\r\n");
        delay_ms(50); // optional
    }
}


