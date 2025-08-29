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
#include <lpc21xx.h>

// Type definitions from the original OCR
typedef unsigned char us;
typedef unsigned short int ushort;

// Function prototypes for I2C and UART
extern void i2c_init(void);
extern void i2c_byte_write_frame(us sa, us mr, us data);
extern us i2c_byte_read_frame(us sa, us mr);
extern void uart0_init(ushort baudrate);
extern void uart0_tx_string(const char *s);
extern void uart0_tx_integer(ushort value);
extern void delay_ms(ushort ms);
extern void lcd_init(void);
extern void lcd_cmd(us command);

// i2c_driver.c
#include <lpc21xx.h>
#include "header.h"

// Macro to check the SI bit in I2CONSET
#define SI ((I2CONSET >> 3) & 1)

// Function to initialize the I2C peripheral
void i2c_init(void) {
    PINSEL0 = 0x50; // Configure P0.2 as SCL and P0.3 as SDA
    I2SCLL = 75;    // Set SCL low time
    I2SCLH = 75;    // Set SCL high time for 100 Kbps with 50% duty cycle
    I2CONSET = (1 << 6); // Enable I2C peripheral
}

// Function to write a byte frame to an I2C slave
void i2c_byte_write_frame(us sa, us mr, us data) {
    /* Generate start condition */
    I2CONSET = (1 << 5); // STA=1
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    I2CONCLR = (1 << 5); // STA=0
    if (I2STAT != 0x8) {
        uart0_tx_string("Err: start condition \r\n");
        goto exit;
    }

    /* Send slave address with write bit & check ACK */
    I2DAT = sa; // SA+W
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x20) {
        uart0_tx_string("Err: SA+w\r\n");
        goto exit;
    }

    /* Send memory address & check ACK */
    I2DAT = mr; // memory address
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x30) {
        uart0_tx_string("Err: m/r addr \r\n");
        goto exit;
    }

    /* Send data & check ACK*/
    I2DAT = data; // data
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x30) {
        uart0_tx_string("Err: data \r\n");
        goto exit;
    }

    /* Generate stop condition */
    exit:
    I2CONSET = (1 << 4); // STO=1
    I2CONCLR = (1 << 3); // clear SI
}

// Function to read a byte frame from an I2C slave
us i2c_byte_read_frame(us sa, us mr) {
    us temp;
    /* Generate start condition */
    I2CONSET = (1 << 5); // STA=1
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    I2CONCLR = (1 << 5); // STA=0
    if (I2STAT != 0x8) {
        uart0_tx_string("Err: start condition \r\n");
        goto exit;
    }

    /* Send slave address with write bit & check ACK */
    I2DAT = sa; // SA+W
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x20) {
        uart0_tx_string("Err: SA+w\r\n");
        goto exit;
    }

    /* Send memory address & check Ack*/
    I2DAT = mr; // memory address
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x30) {
        uart0_tx_string("Err: m/r addr \r\n");
        goto exit;
    }

    /* Generate restart condition */
    I2CONSET = (1 << 5); // STA=1
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    I2CONCLR = (1 << 5); // STA=0
    if (I2STAT != 0x10) {
        uart0_tx_string("Err: Restart condition \r\n");
        goto exit;
    }

    /* Send slave address with read bit & Check ACK */
    I2DAT = sa | 1; // SA+R
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    if (I2STAT == 0x48) {
        uart0_tx_string("Err: SA+r\r\n");
        goto exit;
    }

    /* Read data from the slave & generate No Ack*/
    I2CONCLR = (1 << 3); // clear SI
    while (SI == 0);
    temp = I2DAT; // collect received data

    /* Generate stop condition */
    exit:
    I2CONSET = (1 << 4); // STO=1
    I2CONCLR = (1 << 3); // clear SI
    return temp;
}

// i2c_main.c
#include "header.h"

int main() {
    us temp;
    i2c_init();
    uart0_init(9600);
    uart0_tx_string("I2C Test \r\n");
    i2c_byte_write_frame(0xA0, 0x2, 0x41);
    delay_ms(10);
    temp = i2c_byte_read_frame(0xA0, 0x6);
    return 0;
}


// main-rtc.c
#include "header.h"

int main() {
    us h, m, s;
    i2c_init();
    lcd_init();

    /* set rtc time to HH:MM:SS */
    h = 0x23;
    m = 0x59;
    s = 0x58;

    i2c_byte_write_frame(0xD0, 0x2, h); // set hrs
    i2c_byte_write_frame(0xD0, 0x1, m); // set mins
    i2c_byte_write_frame(0xD0, 0x0, s); // set secs

    /* read rtc time & dump it on the lcd */
    while (1) {
        h = i2c_byte_read_frame(0xD0, 0x2); // read hrs
        m = i2c_byte_read_frame(0xD0, 0x1); // read mins
        s = i2c_byte_read_frame(0xD0, 0x0); // read secs
        lcd_cmd(0x80);
    }
    return 0;
}

