# File: header.h
```c
#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h> // For sprintf, used in uart.c and main.c

// --- Clock Frequencies ---
// Define PCLK_FREQ_HZ based on your SystemInit configuration
// For CCLK = 60MHz (PLLCFG = 0x24) and VPBDIV = 0x00 (PCLK = CCLK/4)
#define PCLK_FREQ_HZ 15000000UL // 15 MHz

// --- Switch Pin Definitions ---
#define SW1 (1 << 9)  // P0.9
#define SW2 (1 << 10) // P0.10
#define SW3 (1 << 11) // P0.11

// --- DS1307 RTC Definitions ---
#define DS1307_I2C_ADDR      0x68 // 7-bit I2C address of DS1307
#define DS1307_REG_SECONDS   0x00
#define DS1307_REG_MINUTES   0x01
#define DS1307_REG_HOURS     0x02
#define DS1307_REG_DAY       0x03
#define DS1307_REG_DATE      0x04
#define DS1307_REG_MONTH     0x05
#define DS1307_REG_YEAR      0x06
#define DS1307_REG_CONTROL   0x07

// --- AT24C08D EEPROM Definitions ---
#define AT24C08D_I2C_BASE_ADDR 0x50 // 7-bit base I2C address (A2,A1,A0 grounded)

// EEPROM Address Map for RTC Data (example)
#define EEPROM_ADDR_SECONDS 0x000
#define EEPROM_ADDR_MINUTES 0x001
#define EEPROM_ADDR_HOURS   0x002
#define EEPROM_ADDR_DAY     0x003
#define EEPROM_ADDR_DATE    0x004
#define EEPROM_ADDR_MONTH   0x005
#define EEPROM_ADDR_YEAR    0x006


// --- Structures ---
typedef struct {
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char day;    // Day of week (1-7, e.g., 1=Sunday)
    unsigned char date;   // Day of month (1-31)
    unsigned char month;  // Month (1-12)
    unsigned char year;   // Year (0-99 for 20xx)
} RTC_Time_TypeDef;


// --- Function Prototypes ---

// delay.c (replaced by timer_delay.c)
void timer0_init(void);
void delay_ms_timer(unsigned int ms);
void delay_sec_timer(unsigned int sec);

// i2c.c
void i2c0_init_master(void);
unsigned char i2c0_start(void);
void i2c0_stop(void);
unsigned char i2c0_send_byte(unsigned char byte);
unsigned char i2c0_read_byte(unsigned char ack);

// lcd.c
void lcd_init(void);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_clear(void);
void lcd_set_cursor(unsigned char row, unsigned char col);
void lcd_puts(char *str);

// rtc_eeprom.c
unsigned char bin_to_bcd(unsigned char bin);
unsigned char bcd_to_bin(unsigned char bcd);
void ds1307_init(void);
void ds1307_set_time(RTC_Time_TypeDef *time);
void ds1307_read_time(RTC_Time_TypeDef *time);
void at24c08_write_byte(unsigned int address, unsigned char data);
unsigned char at24c08_read_byte(unsigned int address);
void at24c08_write_time_to_eeprom(RTC_Time_TypeDef *time);
void at24c08_read_time_from_eeprom(RTC_Time_TypeDef *time);

// uart.c
void uart0_init(unsigned int baud_rate);
void uart0_tx_char(char data);
void uart0_tx_string(char *str);
void uart0_tx_string_newline(char *str);
void uart0_tx_integer(int num);
char uart0_rx_char(void);
// __irq void UART0_ISR(void); // Uncomment if using UART interrupt

#endif // __HEADER_H__
```

# File: i2c.c
```c
#include "header.h"
#include <LPC21xx.H> // This header should contain definitions for I2C registers like I2CONCLR, I2SCLL etc.

#define I2C_TIMEOUT 100000 // A simple timeout for I2C operations

/**
 * @brief Initializes I2C0 as master.
 * Sets I2C clock frequency.
 */
void i2c0_init_master(void) {
    // Select P0.27 (SDA0) and P0.28 (SCL0) for I2C0 functions on LPC2148
    PINSEL1 |= (1 << 23) | (1 << 25); // P0.27 (SDA0) and P0.28 (SCL0)

    // Using I2CONCLR for I2C0 control clear register as per manual
    I2CONCLR = 0x6C; // Clear all I2C interrupt flags and control bits

    // Using I2SCLL and I2SCLH for I2C0 clock speed control
    // Set SCL period for 100kHz (Standard Mode)
    // Formula: I2CxSCLH = I2CxSCLL = PCLK_FREQ_HZ / (2 * I2C_BITRATE_HZ)
    // For PCLK=15MHz, 100kHz: 15,000,000 / (2 * 100,000) = 15,000,000 / 200,000 = 75
    I2SCLL   = 75;
    I2SCLH   = 75;

    // Using I2CONSET for I2C0 control set register
    I2CONSET = (1 << 6); // Enable I2C interface (I2EN)
}

/**
 * @brief Sends an I2C Start condition.
 * @return I2C status code after start condition.
 */
unsigned char i2c0_start(void) {
    unsigned int timeout = I2C_TIMEOUT;
    I2CONSET = (1 << 5); // Set STA (Start) bit
    I2CONCLR = (1 << 3); // Clear SI (Interrupt) flag

    // Using I2CONSET & (1 << 3) to check SI flag, and I2STAT for status
    while (!(I2CONSET & (1 << 3)) && timeout--); // Wait for SI flag
    if (timeout == 0) return 0xFF; // Timeout error

    I2CONCLR = (1 << 5); // Clear STA (Start) bit

    return I2STAT; // Return status code (0x08 for Start, 0x10 for Repeated Start)
}

/**
 * @brief Sends an I2C Stop condition.
 */
void i2c0_stop(void) {
    unsigned int timeout = I2C_TIMEOUT;
    I2CONSET = (1 << 4); // Set STO (Stop) bit
    I2CONCLR = (1 << 3); // Clear SI (Interrupt) flag

    while (I2CONSET & (1 << 4) && timeout--); // Wait until STO is cleared by hardware
    // No SI flag after stop condition in master mode according to user manual
    if (timeout == 0) { /* Handle timeout */ }
}

/**
 * @brief Sends a byte over I2C.
 * @param byte The byte to send.
 * @return I2C status code after sending the byte.
 */
unsigned char i2c0_send_byte(unsigned char byte) {
    unsigned int timeout = I2C_TIMEOUT;
    I2DAT = byte;       // Load byte to Data register
    I2CONCLR = (1 << 3); // Clear SI (Interrupt) flag

    while (!(I2CONSET & (1 << 3)) && timeout--); // Wait for SI flag
    if (timeout == 0) return 0xFF; // Timeout error

    return I2STAT; // Return status code (e.g., 0x18 for SLA+W ACK, 0x28 for Data ACK)
}

/**
 * @brief Reads a byte from I2C.
 * @param ack If 1, send ACK after reading; if 0, send NACK (for last byte).
 * @return The byte read from I2C.
 */
unsigned char i2c0_read_byte(unsigned char ack) {
    unsigned int timeout = I2C_TIMEOUT;
    if (ack) {
        I2CONSET = (1 << 2); // Set AA (Assert Acknowledge) bit
    } else {
        I2CONCLR = (1 << 2); // Clear AA (Assert Acknowledge) bit
    }
    I2CONCLR = (1 << 3); // Clear SI (Interrupt) flag

    while (!(I2CONSET & (1 << 3)) && timeout--); // Wait for SI flag
    if (timeout == 0) return 0xFF; // Timeout error

    return I2DAT; // Return data
}
```

# File: lcd.c
```c

#include "header.h"
#include <LPC21xx.H>

// Assuming LCD control pins are on P0.2 to P0.4
#define LCD_RS (1 << 2) // P0.2
#define LCD_RW (1 << 3) // P0.3
#define LCD_EN (1 << 4) // P0.4

// Assuming LCD data pins are on P0.5 to P0.8 (D4-D7)
#define LCD_D4 (1 << 5)
#define LCD_D5 (1 << 6)
#define LCD_D6 (1 << 7)
#define LCD_D7 (1 << 8)

// LCD Commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME   0x02
#define LCD_ENTRY_MODE_SET 0x06 // Increment cursor, no shift
#define LCD_DISPLAY_ON_OFF 0x0C // Display on, Cursor off, Blink off
#define LCD_FUNCTION_SET   0x28 // 4-bit mode, 2 lines, 5x8 dots (0010 NFxx)

/**
 * @brief Sends a 4-bit nibble to the LCD.
 * @param nibble The 4-bit data/command to send.
 */
void lcd_write_nibble(unsigned char nibble) {
    // Clear previous data on pins (P0.5-P0.8)
    IOCLR0 = (LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7);
    // Set new data on pins (shift nibble to match pin positions)
    IOSET0 = ((nibble & 0x01) << 5) |  // D0 to P0.5
             ((nibble & 0x02) << 5) |  // D1 to P0.6
             ((nibble & 0x04) << 5) |  // D2 to P0.7
             ((nibble & 0x08) << 5);   // D3 to P0.8

    IOSET0 = LCD_EN; // Enable HIGH
    delay_ms_timer(1);     // Short delay (using timer delay)
    IOCLR0 = LCD_EN; // Enable LOW
    delay_ms_timer(1);     // Short delay (using timer delay)
}

/**
 * @brief Sends a command to the LCD.
 * @param cmd The 8-bit command.
 */
void lcd_command(unsigned char cmd) {
    IOCLR0 = LCD_RS; // RS = 0 (Command mode)
    IOCLR0 = LCD_RW; // RW = 0 (Write mode)

    lcd_write_nibble(cmd >> 4); // Send higher nibble
    lcd_write_nibble(cmd & 0x0F); // Send lower nibble
    delay_ms_timer(2); // Commands usually need more time than data (using timer delay)
}

/**
 * @brief Sends data (character) to the LCD.
 * @param data The 8-bit character.
 */
void lcd_data(unsigned char data) {
    IOSET0 = LCD_RS; // RS = 1 (Data mode)
    IOCLR0 = LCD_RW; // RW = 0 (Write mode)

    lcd_write_nibble(data >> 4); // Send higher nibble
    lcd_write_nibble(data & 0x0F); // Send lower nibble
    delay_ms_timer(1); // Data write usually faster (using timer delay)
}

/**
 * @brief Initializes the 16x2 LCD in 4-bit mode.
 */
void lcd_init(void) {
    // Configure LCD pins as output (P0.2 - P0.8)
    IODIR0 |= (LCD_RS | LCD_RW | LCD_EN | LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7);
    IOCLR0 = (LCD_RS | LCD_RW | LCD_EN); // Ensure control lines are low initially

    delay_ms_timer(100); // Power-on delay for LCD (using timer delay)

    // --- 4-bit initialization sequence ---
    lcd_write_nibble(0x03); // Send 0x03 (first part of function set)
    delay_ms_timer(5);

    lcd_write_nibble(0x03); // Send 0x03 again
    delay_ms_timer(1);

    lcd_write_nibble(0x03); // Send 0x03 a third time
    delay_ms_timer(1);

    lcd_write_nibble(0x02); // Set to 4-bit interface
    delay_ms_timer(1);

    // --- Functional Set (now in 4-bit mode) ---
    lcd_command(LCD_FUNCTION_SET);   // 4-bit, 2 lines, 5x8 dots
    lcd_command(LCD_DISPLAY_ON_OFF); // Display ON, Cursor OFF, Blink OFF
    lcd_command(LCD_CLEAR_DISPLAY);  // Clear display
    delay_ms_timer(2); // Clear display needs longer delay (using timer delay)
    lcd_command(LCD_ENTRY_MODE_SET); // Entry mode: increment cursor, no shift
}

/**
 * @brief Clears the LCD display.
 */
void lcd_clear(void) {
    lcd_command(LCD_CLEAR_DISPLAY);
    delay_ms_timer(2); // Using timer delay
}

/**
 * @brief Sets the cursor position on the LCD.
 * @param row Row number (0 or 1).
 * @param col Column number (0-15).
 */
void lcd_set_cursor(unsigned char row, unsigned char col) {
    unsigned char address;
    if (row == 0) {
        address = 0x00 + col;
    } else {
        address = 0x40 + col;
    }
    lcd_command(0x80 | address); // Set DDRAM Address command (DB7=1)
}

/**
 * @brief Prints a string to the LCD at the current cursor position.
 * @param str The string to print.
 */
void lcd_puts(char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}
```

# File: main.c
```c

#include <LPC21xx.H>
#include "header.h"
#include <stdio.h> // For sprintf
#include <string.h> // For strcpy, strlen

// Extern declarations for global variables from uart.c
extern volatile char received_char_isr;
extern volatile unsigned char new_data_flag;

// --- Global Variables for Clock and Setting Mode ---
RTC_Time_TypeDef current_time;
volatile unsigned char setting_mode = 0; // 0: Display, 1: Set Hours, 2: Set Minutes, 3: Set Seconds
volatile unsigned char sw1_pressed = 0; // Flag for SW1 press
volatile unsigned char sw2_pressed = 0; // Flag for SW2 press
volatile unsigned char sw3_pressed = 0; // Flag for SW3 press

// --- Debounce Function for Switches ---
// Assumes switches are connected as active-low (pull-up to VCC, switch to GND)
unsigned char debounce_switch(unsigned long pin_mask) {
    if (!(IOPIN0 & pin_mask)) { // Check if switch is pressed (pin is low)
        delay_ms_timer(50); // Debounce delay (using timer delay)
        if (!(IOPIN0 & pin_mask)) { // Confirm press
            while (!(IOPIN0 & pin_mask)); // Wait for release
            delay_ms_timer(50); // Debounce release (using timer delay)
            return 1; // Switch was pressed and released
        }
    }
    return 0; // Switch not pressed
}

// System Initialization function
void SystemInit(void) {
    PLLCFG = 0x24;   // M = 5, P = 2
    PLLCON = 0x01;   // Enable PLL
    PLLFEED = 0xAA;  // Feed sequence for PLL register update
    PLLFEED = 0x55;

    // Wait for PLL to Lock (PLOCK bit 10 in PLLSTAT)
    while (!(PLLSTAT & (1 << 10)));

    PLLCON = 0x03;   // Connect PLL and Enable PLL
    PLLFEED = 0xAA;  // Feed sequence
    PLLFEED = 0x55;

    // Configure VPB Divider for PCLK = CCLK / 4 = 15MHz
    VPBDIV = 0x00;   // VPBDIV = 0x00 for CCLK/4
}

int main(void) {
    // --- Moved all variable declarations to the top of main function ---
    char lcd_buffer[17]; // 16 chars + null terminator
    unsigned char display_hours_12hr;
    unsigned char is_am; // 1 for AM, 0 for PM
    unsigned char raw_hours_bin; // Declaration moved
    char param_label[3]; // "HR", "MN", "SC" - Declaration moved
    unsigned char param_val; // Declaration moved
    int i; // Loop variable declaration moved

    SystemInit();
    uart0_init(9600);
    i2c0_init_master();   // Initialize I2C for DS1307 and AT24C08D
    lcd_init();           // Initialize LCD
    ds1307_init();        // Initialize DS1307 RTC

    // --- ADD THIS LINE TO INITIALIZE TIMER0 ---
    timer0_init(); // Initialize Timer0 for delays

    // --- Configure Switch Pins as Inputs ---
    // P0.9 (SW1), P0.10 (SW2), P0.11 (SW3)
    IODIR0 &= ~(SW1 | SW2 | SW3); // Set as input (clear bits)
    // External pull-up resistors for switches should be used in Proteus.

    uart0_tx_string_newline("Digital Clock with RTC & EEPROM");
    uart0_tx_string_newline("-------------------------------");
    uart0_tx_string_newline("SW1: Mode | SW2: Next | SW3: Inc");
    delay_sec_timer(1); // Using timer delay

    // Try to read time from EEPROM first
    at24c08_read_time_from_eeprom(&current_time);

    // If EEPROM read gives invalid time (e.g., all 0xFF from uninitialized EEPROM),
    // or if the time is clearly out of range, set a default time to RTC and save to EEPROM.
    // Assuming binary values for comparison after conversion from BCD.
    if (bcd_to_bin(current_time.hours) > 23 || bcd_to_bin(current_time.minutes) > 59 ||
        bcd_to_bin(current_time.seconds) > 59 || bcd_to_bin(current_time.month) > 12 ||
        bcd_to_bin(current_time.date) > 31 || bcd_to_bin(current_time.day) > 7) {

        uart0_tx_string_newline("EEPROM time invalid/empty. Setting default RTC time.");
        current_time.seconds = 0;
        current_time.minutes = 0;
        current_time.hours = 12; // Default to 12:00:00 (24-hour format)
        current_time.day = 1; // Sunday
        current_time.date = 1;
        current_time.month = 1;
        current_time.year = 24; // Year 2024

        ds1307_set_time(&current_time); // Set default time to RTC
        at24c08_write_time_to_eeprom(&current_time); // Save default time to EEPROM
        delay_ms_timer(100); // Small delay after EEPROM write (using timer delay)
    } else {
        // Valid time read from EEPROM, set it to RTC
        uart0_tx_string_newline("Time loaded from EEPROM to RTC.");
        ds1307_set_time(&current_time);
        delay_ms_timer(100); // Using timer delay
    }

    lcd_clear();

    while (1) {
        // --- Read Time from RTC ---
        ds1307_read_time(&current_time);

        // --- Handle Switch Inputs ---
        if (debounce_switch(SW1)) {
            setting_mode++;
            if (setting_mode > 3) { // Cycle through 0:Display, 1:Hours, 2:Minutes, 3:Seconds
                setting_mode = 0;
                // When exiting setting mode, save current RTC time to EEPROM
                at24c08_write_time_to_eeprom(&current_time);
                uart0_tx_string_newline("Time saved to EEPROM.");
            }
            uart0_tx_string("Mode: "); uart0_tx_integer(setting_mode); uart0_tx_string_newline("");
            lcd_clear(); // Clear LCD on mode change
        }

        if (setting_mode > 0) { // Only process SW2/SW3 if in setting mode
            if (debounce_switch(SW2)) {
                setting_mode++; // Move to next parameter
                if (setting_mode > 3) setting_mode = 1; // Wrap around (Hours -> Minutes -> Seconds -> Hours)
                uart0_tx_string("Next Setting: Mode "); uart0_tx_integer(setting_mode); uart0_tx_string_newline("");
                lcd_clear();
            }
            if (debounce_switch(SW3)) {
                switch (setting_mode) {
                    case 1: // Set Hours
                        current_time.hours = bcd_to_bin(current_time.hours);
                        current_time.hours = (current_time.hours + 1) % 24; // 0-23
                        current_time.hours = bin_to_bcd(current_time.hours);
                        break;
                    case 2: // Set Minutes
                        current_time.minutes = bcd_to_bin(current_time.minutes);
                        current_time.minutes = (current_time.minutes + 1) % 60; // 0-59
                        current_time.minutes = bin_to_bcd(current_time.minutes);
                        break;
                    case 3: // Set Seconds (often set to 0 when entering setting mode)
                        current_time.seconds = 0; // Reset seconds to 0 when entering seconds setting
                        break;
                }
                // Update RTC immediately after incrementing
                ds1307_set_time(&current_time);
                uart0_tx_string_newline("Value Incremented.");
            }
        }

        // --- Prepare Time for 12-hour AM/PM Display ---
        raw_hours_bin = bcd_to_bin(current_time.hours); // Declaration moved
        display_hours_12hr = raw_hours_bin;

        if (display_hours_12hr >= 12) {
            is_am = 0; // PM
            if (display_hours_12hr > 12) {
                display_hours_12hr -= 12;
            }
        } else {
            is_am = 1; // AM
            if (display_hours_12hr == 0) {
                display_hours_12hr = 12; // 00:XX AM is 12:XX AM
            }
        }

        // --- Display on LCD based on Mode ---
        lcd_set_cursor(0, 0); // Always put current time on top line

        // Format time as HH:MM:SS AM/PM on Line 1
        sprintf(lcd_buffer, "%02d:%02d:%02d %s",
                display_hours_12hr,
                bcd_to_bin(current_time.minutes),
                bcd_to_bin(current_time.seconds),
                is_am ? "AM" : "PM");
        lcd_puts(lcd_buffer);
        // Pad with spaces if the string is shorter than 16 characters
        for (i = strlen(lcd_buffer); i < 16; i++) { // Corrected loop variable declaration
            lcd_data(' ');
        }

        lcd_set_cursor(1, 0); // Second line for date or setting parameter

        if (setting_mode == 0) { // Display Mode: show date
            // Format date as DD/MM/20YY (assuming year is 20YY)
            sprintf(lcd_buffer, "%02d/%02d/20%02d",
                    bcd_to_bin(current_time.date),
                    bcd_to_bin(current_time.month),
                    bcd_to_bin(current_time.year));
            lcd_puts(lcd_buffer);
            // Pad with spaces
            for (i = strlen(lcd_buffer); i < 16; i++) { // Corrected loop variable declaration
                lcd_data(' ');
            }
        } else { // Setting Mode: show setting parameter
            // Declarations moved to top of main
            // char param_label[3];
            // unsigned char param_val;

            switch (setting_mode) {
                case 1:
                    strcpy(param_label, "HR");
                    param_val = bcd_to_bin(current_time.hours);
                    break;
                case 2:
                    strcpy(param_label, "MN");
                    param_val = bcd_to_bin(current_time.minutes);
                    break;
                case 3:
                    strcpy(param_label, "SC");
                    param_val = bcd_to_bin(current_time.seconds);
                    break;
                default: // Should not happen with current setting_mode range
                    strcpy(param_label, "");
                    param_val = 0;
                    break;
            }
            sprintf(lcd_buffer, "SET %s %02d", param_label, param_val);
            lcd_puts(lcd_buffer);
            // Pad with spaces
            for (i = strlen(lcd_buffer); i < 16; i++) { // Corrected loop variable declaration
                lcd_data(' ');
            }
        }

        delay_ms_timer(100); // Small delay for display update rate (using timer delay)
    }
}
```

# File: rtc_eeprom.c
```c
#include "header.h"

// --- BCD Conversion Functions ---
unsigned char bin_to_bcd(unsigned char bin) {
    return ((bin / 10) << 4) | (bin % 10);
}

unsigned char bcd_to_bin(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// --- DS1307 RTC Functions ---

/**
 * @brief Initializes the DS1307 RTC.
 * Ensures the oscillator is running (CH bit cleared) and sets 24-hour mode.
 */
void ds1307_init(void) {
    unsigned char status;
    // --- Moved declarations to the top of the function ---
    unsigned char seconds_reg, hours_reg; // Moved from line 56
    // unsigned char control_reg; // Removed as it was set but never used

    // First, try to read the seconds and hours registers to check CH bit and 12/24 mode.
    // Set register pointer to Seconds (0x00)
    i2c0_start();
    status = i2c0_send_byte((DS1307_I2C_ADDR << 1) | 0); // DS1307 address + Write
    if (status != 0x18) { /* Handle error */ i2c0_stop(); return; }

    status = i2c0_send_byte(DS1307_REG_SECONDS); // Send Seconds Register address
    if (status != 0x28) { /* Handle error */ i2c0_stop(); return; }
    i2c0_stop();

    // Now, read seconds and hours
    i2c0_start();
    status = i2c0_send_byte((DS1307_I2C_ADDR << 1) | 1); // DS1307 address + Read
    if (status != 0x40) { /* Handle error */ i2c0_stop(); return; }

    seconds_reg = i2c0_read_byte(1); // Read seconds, send ACK for more bytes
    hours_reg = i2c0_read_byte(0);   // Read hours, send NACK (last byte for this read)
    i2c0_stop();

    // Set control register to 0x00 (disable SQW/OUT)
    i2c0_start();
    i2c0_send_byte((DS1307_I2C_ADDR << 1) | 0); // DS1307 address + Write
    i2c0_send_byte(DS1307_REG_CONTROL);       // Control Register address
    i2c0_send_byte(0x00);                     // Write 0x00 to Control Register
    i2c0_stop();
    delay_ms_timer(10); // Small delay for write cycle (using timer delay)

    // If Clock Halt (CH) bit is set (bit 7 of Seconds register), clear it to start oscillator
    // Or if hours_reg is in 12-hour mode (bit 6 set), set to 24-hour mode.
    if ((seconds_reg & 0x80) || (hours_reg & 0x40)) { // CH bit (bit 7) or 12-hour mode (bit 6)
        RTC_Time_TypeDef initial_time;
        initial_time.seconds = 0; // Clear CH bit
        initial_time.minutes = 0;
        initial_time.hours = 0;   // 24-hour mode (0-23)
        initial_time.day = 1;
        initial_time.date = 1;
        initial_time.month = 1;
        initial_time.year = 0; // Year 2000

        ds1307_set_time(&initial_time); // Set a default time to start oscillator
    }
}


/**
 * @brief Sets the time and date in the DS1307 RTC.
 * Time values should be in Binary, converted to BCD internally.
 * @param time Pointer to RTC_Time_TypeDef structure containing time/date.
 */
void ds1307_set_time(RTC_Time_TypeDef *time) {
    unsigned char status;

    i2c0_start();
    status = i2c0_send_byte((DS1307_I2C_ADDR << 1) | 0); // DS1307 address + Write
    if (status != 0x18) { /* Handle error */ return; }

    status = i2c0_send_byte(DS1307_REG_SECONDS); // Start writing from Seconds register
    if (status != 0x28) { /* Handle error */ return; }

    // Write time/date registers in BCD format
    i2c0_send_byte(bin_to_bcd(time->seconds) & 0x7F); // Clear CH bit (bit 7)
    i2c0_send_byte(bin_to_bcd(time->minutes));
    i2c0_send_byte(bin_to_bcd(time->hours) & 0x3F);   // Ensure 24-hour mode (bit 6=0, bit 5=0)
    i2c0_send_byte(bin_to_bcd(time->day));
    i2c0_send_byte(bin_to_bcd(time->date));
    i2c0_send_byte(bin_to_bcd(time->month));
    i2c0_send_byte(bin_to_bcd(time->year));

    i2c0_stop();
    delay_ms_timer(10); // Small delay for write cycle (using timer delay)
}

/**
 * @brief Reads the time and date from the DS1307 RTC.
 * Time values are read as BCD and converted to Binary.
 * @param time Pointer to RTC_Time_TypeDef structure to store time/date.
 */
void ds1307_read_time(RTC_Time_TypeDef *time) {
    unsigned char status;

    // Set register pointer to Seconds (0x00)
    i2c0_start();
    status = i2c0_send_byte((DS1307_I2C_ADDR << 1) | 0); // DS1307 address + Write
    if (status != 0x18) { /* Handle error */ return; }

    status = i2c0_send_byte(DS1307_REG_SECONDS); // Send Seconds Register address
    if (status != 0x28) { /* Handle error */ return; }
    i2c0_stop();

    // Now, read 7 bytes starting from Seconds
    i2c0_start();
    status = i2c0_send_byte((DS1307_I2C_ADDR << 1) | 1); // DS1307 address + Read
    if (status != 0x40) { /* Handle error */ return; }

    time->seconds = bcd_to_bin(i2c0_read_byte(1) & 0x7F); // Read seconds (mask CH bit)
    time->minutes = bcd_to_bin(i2c0_read_byte(1));       // Read minutes
    time->hours   = bcd_to_bin(i2c0_read_byte(1) & 0x3F); // Read hours (mask 12/24 mode and AM/PM bits)
    time->day     = bcd_to_bin(i2c0_read_byte(1));       // Read day
    time->date    = bcd_to_bin(i2c0_read_byte(1));       // Read date
    time->month   = bcd_to_bin(i2c0_read_byte(1));       // Read month
    time->year    = bcd_to_bin(i2c0_read_byte(0));       // Read year, send NACK (last byte)
    i2c0_stop();
}

// --- AT24C08D EEPROM Functions ---

/**
 * @brief Writes a single byte to the AT24C08D EEPROM.
 * @param address 10-bit memory address (0x000 to 0x3FF).
 * @param data The byte to write.
 */
void at24c08_write_byte(unsigned int address, unsigned char data) {
    unsigned char status;
    // AT24C08D uses A9 and A8 bits in the 7-bit device address for memory block selection.
    // The device address byte is 1010 A2 A9 A8 R/W.
    // A2 is assumed to be 0 (grounded).
    // A9 is bit 9 of the address, A8 is bit 8 of the address.
    unsigned char device_address_byte = (AT24C08D_I2C_BASE_ADDR << 1) | ((address >> 7) & 0x06); // A9, A8 bits
    device_address_byte |= 0; // Write bit

    i2c0_start();
    status = i2c0_send_byte(device_address_byte); // Send Device Address + A9/A8 + Write bit
    if (status != 0x18) { /* Handle error */ i2c0_stop(); return; } // SLA+W transmitted, ACK received

    status = i2c0_send_byte(address & 0xFF); // Send 8-bit Word Address (A7-A0)
    if (status != 0x28) { /* Handle error */ i2c0_stop(); return; } // Data transmitted, ACK received

    status = i2c0_send_byte(data); // Send Data Byte
    if (status != 0x28) { /* Handle error */ i2c0_stop(); return; } // Data transmitted, ACK received
    i2c0_stop();

    // Acknowledge Polling: Wait for write cycle to complete (WIP bit clears)
    // This is done by repeatedly sending START + SLA+W. EEPROM will ACK when ready.
    do {
        i2c0_start();
        status = i2c0_send_byte(device_address_byte);
    } while (status != 0x18); // Loop until SLA+W is ACKed
    i2c0_stop();
}

/**
 * @brief Reads a single byte from the AT24C08D EEPROM.
 * @param address 10-bit memory address (0x000 to 0x3FF).
 * @return The byte read from EEPROM.
 */
unsigned char at24c08_read_byte(unsigned int address) {
    unsigned char data_read;
    unsigned char status;
    // AT24C08D uses A9 and A8 bits in the 7-bit device address for memory block selection.
    unsigned char device_address_byte_write = (AT24C08D_I2C_BASE_ADDR << 1) | ((address >> 7) & 0x06);
    unsigned char device_address_byte_read = device_address_byte_write | 1; // Set Read bit

    // Dummy Write to set internal address pointer
    i2c0_start();
    status = i2c0_send_byte(device_address_byte_write); // Send Device Address + A9/A8 + Write bit
    if (status != 0x18) { /* Handle error */ i2c0_stop(); return 0xFF; }

    status = i2c0_send_byte(address & 0xFF); // Send 8-bit Word Address (A7-A0)
    if (status != 0x28) { /* Handle error */ i2c0_stop(); return 0xFF; }
    // No STOP after dummy write, instead send REPEATED START

    // Repeated START for Read
    status = i2c0_start(); // Repeated START
    if (status != 0x10) { /* Handle error */ i2c0_stop(); return 0xFF; } // 0x10: Repeated START transmitted

    status = i2c0_send_byte(device_address_byte_read); // Send Device Address + A9/A8 + Read bit
    if (status != 0x40) { /* Handle error */ i2c0_stop(); return 0xFF; } // SLA+R transmitted, ACK received

    data_read = i2c0_read_byte(0); // Read byte, send NACK (as it's the last byte)
    i2c0_stop();
    return data_read;
}

/**
 * @brief Writes current RTC time to AT24C08D EEPROM.
 * @param time Pointer to RTC_Time_TypeDef structure.
 */
void at24c08_write_time_to_eeprom(RTC_Time_TypeDef *time) {
    at24c08_write_byte(EEPROM_ADDR_SECONDS, time->seconds);
    at24c08_write_byte(EEPROM_ADDR_MINUTES, time->minutes);
    at24c08_write_byte(EEPROM_ADDR_HOURS,   time->hours);
    at24c08_write_byte(EEPROM_ADDR_DAY,     time->day);
    at24c08_write_byte(EEPROM_ADDR_DATE,    time->date);
    at24c08_write_byte(EEPROM_ADDR_MONTH,   time->month);
    at24c08_write_byte(EEPROM_ADDR_YEAR,    time->year);
}

/**
 * @brief Reads RTC time from AT24C08D EEPROM.
 * @param time Pointer to RTC_Time_TypeDef structure to store data.
 */
void at24c08_read_time_from_eeprom(RTC_Time_TypeDef *time) {
    time->seconds = at24c08_read_byte(EEPROM_ADDR_SECONDS);
    time->minutes = at24c08_read_byte(EEPROM_ADDR_MINUTES);
    time->hours   = at24c08_read_byte(EEPROM_ADDR_HOURS);
    time->day     = at24c08_read_byte(EEPROM_ADDR_DAY);
    time->date    = at24c08_read_byte(EEPROM_ADDR_DATE);
    time->month   = at24c08_read_byte(EEPROM_ADDR_MONTH);
    time->year    = at24c08_read_byte(EEPROM_ADDR_YEAR);
}
```


# File: timer_delay.c
```c

#include "header.h"
#include <LPC21xx.H>

/**
 * @brief Initializes Timer0 for delay operations.
 * Sets the Presale Register (PR) to achieve a 1us tick.
 * PCLK_FREQ_HZ must be defined in header.h (e.g., 15000000 for 15MHz).
 */
void timer0_init(void) {
    // Power up Timer0 (bit 1 of PCONP)
    PCONP |= (1 << 1);

    // Reset Timer Control Register (TCR) to disable and reset
    T0TCR = 0x00;

    // Set Presale Register (PR) to get a 1 microsecond (us) increment
    // PR value = (PCLK_FREQ_HZ / 1000000) - 1
    // For PCLK = 15MHz, PR = (15,000,000 / 1,000,000) - 1 = 15 - 1 = 14
    T0PR = (PCLK_FREQ_HZ / 1000000UL) - 1; // PCLK_FREQ_HZ / 1MHz - 1

    // Clear any pending interrupt flags
    T0IR = 0xFFFFFFFF; // Write 1s to clear all flags
}

/**
 * @brief Provides a delay in milliseconds using Timer0.
 * This is a blocking delay.
 * @param ms The number of milliseconds to delay.
 */
void delay_ms_timer(unsigned int ms) {
    // 1. Reset the Timer Counter (TC) and Prescale Counter (PC)
    T0TCR = 0x02; // Bit 1 for Reset

    // 2. Clear any pending interrupt flags for Timer0 (specifically Match Channel 0)
    T0IR = (1 << 0); // Clear MR0 interrupt flag

    // 3. Set the Match Register (MR0) for the desired delay duration
    // Each tick is 1us, so for 'ms' milliseconds, we need 'ms * 1000' ticks.
    T0MR0 = (unsigned long)ms * 1000UL;

    // 4. Configure Match Control Register (MCR)
    // Bit 0: Interrupt on MR0 (MR0I)
    // Bit 1: Reset on MR0 (MR0R)
    T0MCR = (1 << 0) | (1 << 1);

    // 5. Enable Timer0 (start counting)
    T0TCR = 0x01; // Bit 0 for Enable

    // 6. Wait for the Match 0 Interrupt Flag (MR0I) to be set
    // This indicates that the timer has reached T0MR0
    while (!(T0IR & (1 << 0)));

    // 7. Clear the Match 0 Interrupt Flag
    T0IR = (1 << 0);

    // 8. Disable Timer0
    T0TCR = 0x00;
}

/**
 * @brief Provides a delay in seconds using Timer0.
 * @param sec The number of seconds to delay.
 */
void delay_sec_timer(unsigned int sec) {
    unsigned int i;
    for (i = 0; i < sec; i++) {
        delay_ms_timer(1000); // 1 second = 1000 milliseconds
    }
}
```


# File: uart.c
```c
#include "header.h"
#include <LPC21xx.H>

// Global variables for UART receive
volatile char received_char_isr;
volatile unsigned char new_data_flag = 0;

/**
 * @brief Initializes UART0.
 * @param baud_rate The desired baud rate.
 */
void uart0_init(unsigned int baud_rate) {
    unsigned int U0DLL_val;

    // Set P0.0 and P0.1 as TXD0 and RXD0 respectively
    PINSEL0 |= (1 << 0) | (1 << 2); // P0.0 as TXD0, P0.1 as RXD0

    U0LCR = (1 << 7); // Enable DLAB (Divisor Latch Access Bit) to set baud rate
    U0DLL_val = (PCLK_FREQ_HZ / (16 * baud_rate)); // Calculate U0DLL
    U0DLL = U0DLL_val;
    U0DLM = (U0DLL_val >> 8); // U0DLM is high byte (usually 0 for common baud rates)

    U0LCR = (3 << 0); // 8-bit word length, 1 stop bit, disable DLAB
    U0FCR = (1 << 0) | (1 << 1) | (1 << 2); // Enable FIFO, reset RX and TX FIFO

    // Enable UART0 RX Interrupt (optional, but useful for interactive systems)
    // U0IER = (1 << 0); // Enable RBR Interrupt (Receive Buffer Register Full)
    // VICVectAddr4 = (unsigned long)UART0_ISR; // Set ISR address for UART0
    // VICVectCntl4 = (1 << 5) | 6; // Enable UART0 IRQ (Source 6)
    // VICIntEnable = (1 << 6); // Enable UART0 in VIC
}

/**
 * @brief Transmits a single character over UART0.
 * @param data The character to transmit.
 */
void uart0_tx_char(char data) {
    while (!(U0LSR & (1 << 5))); // Wait until THR is empty (TX ready)
    U0THR = data;
}

/**
 * @brief Transmits a string over UART0.
 * @param str The string to transmit.
 */
void uart0_tx_string(char *str) {
    while (*str) {
        uart0_tx_char(*str++);
    }
}

/**
 * @brief Transmits a string followed by a newline over UART0.
 * @param str The string to transmit.
 */
void uart0_tx_string_newline(char *str) {
    uart0_tx_string(str);
    uart0_tx_char('\r'); // Carriage Return
    uart0_tx_char('\n'); // Line Feed
}

/**
 * @brief Transmits an integer over UART0.
 * @param num The integer to transmit.
 */
void uart0_tx_integer(int num) {
    char buffer[12]; // Enough for -2,147,483,648 plus null terminator
    sprintf(buffer, "%d", num);
    uart0_tx_string(buffer);
}

/**
 * @brief Receives a single character from UART0.
 * @return The received character.
 */
char uart0_rx_char(void) {
    while (!(U0LSR & (1 << 0))); // Wait until RDR is full (RX data ready)
    return U0RBR;
}

/*
// --- UART0 Interrupt Service Routine (ISR) Example ---
// Make sure to set up VICVectAddr4 and VICVectCntl4 in uart0_init
// Add this to your project's startup.s if using
__irq void UART0_ISR(void) {
    unsigned char IIR_value = U0IIR; // Read IIR to clear interrupt and find source

    // Check interrupt ID bits (IID)
    switch ((IIR_value >> 1) & 0x07) { // Mask bits 1-3
        case 0x02: // THRE Interrupt (Transmit Holding Register Empty)
            // Handle TX if needed
            break;
        case 0x04: // RBR Interrupt (Receive Buffer Register Full)
            received_char_isr = U0RBR; // Read received char
            new_data_flag = 1; // Set flag to indicate new data
            break;
        case 0x06: // RX Line Status Interrupt
            // Handle errors like Overrun, Parity, Framing, Break
            break;
        case 0x0C: // Character Timeout Interrupt
            // RX FIFO is not empty but no char has been received for 1.5 to 3.5 char times
            break;
    }

    VICVectAddr = 0; // Acknowledge interrupt in VIC
}
*/
