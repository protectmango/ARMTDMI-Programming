// header.h
#ifndef HEADER_H
#define HEADER_H

#include <lpc21xx.h>
#include <stdint.h>
#include <stdbool.h>

// --- PLL and Clock Configuration ---
#define FOSC    10000000    // External crystal frequency
#define CCLK    60000000    // Core clock frequency
#define PCLK    (CCLK / 4)  // Peripheral clock frequency

// --- System Initialization Function Prototypes ---
void pll_init(void);
void vic_init(void);

// --- SPI Driver Function Prototypes ---
#define SSEL_PIN      (1 << 4)  // P0.4
#define SCK_PIN       (1 << 7)  // P0.7
#define MISO_PIN      (1 << 5)  // P0.5
#define MOSI_PIN      (1 << 6)  // P0.6

void spi_init(void);
uint8_t spi_transfer_byte(uint8_t data);
void spi_send_data(uint8_t* tx_data, uint32_t len);

// --- Timer Driver Function Prototypes ---
#define TIMER_INTERVAL_MS 1000 // Log data every 1 second

void timer1_init(void);
__irq void Timer1_ISR(void);
void log_data_and_send(void);

// --- ENC28J60 Driver Function Prototypes ---
// SPI Instructions
#define WBM_OPCODE    0x7A // Write Buffer Memory (WBM)
#define BFS_OPCODE    0xA0 // Bit Field Set (BFS)
#define BFC_OPCODE    0x90 // Bit Field Clear (BFC)
#define RCR_OPCODE    0x00 // Read Control Register (RCR)
#define WCR_OPCODE    0x40 // Write Control Register (WCR)

// Control Register Addresses
#define ECON1_REG     0x1F // ECON1 register address
#define ECON1_TXRTS   (1 << 3) // TXRTS bit in ECON1 (Transmit Request)

void enc28j60_init(void);
void enc28j60_send_packet(const uint8_t* data, uint16_t len);
void enc28j60_write_register(uint8_t reg, uint8_t value);
void enc28j60_bit_field_set(uint8_t reg, uint8_t mask);
void enc28j60_bit_field_clear(uint8_t reg, uint8_t mask);
void enc28j60_chip_select_high(void);
void enc28j60_chip_select_low(void);

#endif // HEADER_H


