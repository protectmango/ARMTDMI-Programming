#include "header.h"

/*SPI Register's*/

/*
S0SPCR : SPI Control Register

Bit 7-3 are used for :
	SPIE : SPI interrupt enable.(bit 7)
	LSBF : LSB First Control.(bit 6)
	MSTR : Master Mode Select.(bit 5)
	CPOL : Clock Polarity.(bit 4)
	CPAH : Clock Phase.(bit 3)

S0SPCCR : SPI Clock Control Register

Control the frequency of the clock

for 100 kbps, set the value 0x150.
for 1 mbps,  set the value 0x15.

S0SPDR : SPI Data Register

This is the master shift register.
Writing to this register start the SPI transfer.
Data transmitted by the slave is received and stored in this register.

S0SPSR : SPI Status Register

Contains Flag:

SPIF : SPI Transfer Complete flag.(bit 7)
WCOL : Write Collision Flag. (bit 6)
ROUR : Read Overrun Flag.(bit 5)
MODF : Mode Fault Flag.(bit 4)
ABRT : Slave Abort Flag.(bit 3)

 */

#include < lpc21xx.h>
#include "header.h"

void spio-init(void) {
    PINSEL0 = 0x1500; // P0.4 -> SCKO, P0.5 -> MISOO, P0.6 -> MOSI
    IODIR0 |= CSO; // P0.7 is output direction
    IOSET0 = CSO; // deselect slave
    S0SPCR = 0x20; // CPOL=CPHA=0, select master mode, MSB first
    S0SPCCR = 0x15; // SPI Clock is 1 Mbps
}

us spio(us data) {
    SOSPDR = data;
    while(SPIF == 0); // waiting for SPI transfer
    return SOSPDR;
}


/*
MCP3209 External ADC 
- 12 bit resolution.
- 4 Analog Input channel.
- Can operate in SPI Mode 0 and 3.


*/
// header.h
#define CSO (1<<7)
extern void spio-init(void);
typedef unsigned short int ushort;
typedef unsigned char us;
us spio(us data);
ushort read_mcp3204(us ch_num);

// mcp3204-driver.c
#include <lpc21xx.h>
#include "header.h"

ushort read_mcp3204(us ch_num) {
    us byteL = 0, byteH = 0;
    ushort result = 0;
    ch_num <<= 6; // set the ch-num

    IOCLR0 = CSO; // select slave
    spio(0x06); // start bit, SGL mode, D2
    byteH = spio(ch_num); // CH select
    byteL = spio(0x00);
    IOSET0 = CSO; // deselect slave

    byteH &= 0x0F; // mask higher nibble
    result = (byteH << 8) | byteL; // merge result
    return result;
}

// main_mcp3204.C
#include "header.h"

main() {
    ushort temp;
    uarto-init(9600);
    spio-init();
    uarto-tx-string("MCP3204 Testing \r\n");

    while(1) {
        temp = read_mcp3204(0); // read CH0
        uarto-tx-integer(temp);
        uarto-tx-string("\r\n");
        delay_ms(50); // optional
    }
}
