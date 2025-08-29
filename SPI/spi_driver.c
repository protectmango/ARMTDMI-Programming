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



/*
MCP3209 External ADC 
- 12 bit resolution.
- 4 Analog Input channel.
- Can operate in SPI Mode 0 and 3.


*/

