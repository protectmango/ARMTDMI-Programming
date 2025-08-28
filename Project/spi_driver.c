// spi_driver.c
#include "header.h"

// Initialize SPI0 on LPC2129
void spi_init(void) {
    // Configure SPI0 pins on LPC2129
    // P0.4 (SSEL0), P0.5 (MISO0), P0.6 (MOSI0), P0.7 (SCK0)
    PINSEL0 |= (1 << 8) | (1 << 10) | (1 << 12) | (1 << 14); 
    
    // Configure SPI0 Control Register (S0SPCR)
    // SPCR is used to control various SPI features.
    S0SPCR = (1 << 5); // Master mode
    S0SPCR &= ~((1 << 4) | (1 << 3)); // CPOL=0, CPHA=0 (mode 0)
    
    // Configure SPI Clock Counter Register (S0SPCCR)
    // This register determines the SPI clock rate.
    S0SPCCR = 8; // SPI clock = PCLK / 8
}

// Transfer a single byte over SPI and return the received byte
uint8_t spi_transfer_byte(uint8_t data) {
    S0SPDR = data; // Write byte to SPI Data Register
    while (!(S0SPSR & (1 << 7))); // Wait for SPIF (SPI Transfer Complete) flag
    return S0SPDR; // Read and return received byte
}

// Send a block of data over SPI
void spi_send_data(uint8_t* tx_data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        spi_transfer_byte(tx_data[i]);
    }
}

