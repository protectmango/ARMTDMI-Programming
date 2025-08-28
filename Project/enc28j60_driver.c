// enc28j60_driver.c
#include "header.h"
#include <string.h>

// Helper functions for ENC28J60 SPI communication
void enc28j60_chip_select_low(void) {
    // Drive the Slave Select pin low to select the ENC28J60
    IOCLR0 = SSEL_PIN;
}

void enc28j60_chip_select_high(void) {
    // Drive the Slave Select pin high to deselect the ENC28J60
    IOSET0 = SSEL_PIN;
}

// Write to a control register on the ENC28J60
void enc28j60_write_register(uint8_t reg, uint8_t value) {
    enc28j60_chip_select_low();
    // Send the Write Control Register opcode (WCR) with the 5-bit register address
    spi_transfer_byte(WCR_OPCODE | (reg & 0x1F));
    // Send the data byte to write to the register
    spi_transfer_byte(value);
    enc28j60_chip_select_high();
}

// Set bits in a control register
void enc28j60_bit_field_set(uint8_t reg, uint8_t mask) {
    enc28j60_chip_select_low();
    // Send the Bit Field Set opcode (BFS) with the 5-bit register address
    spi_transfer_byte(BFS_OPCODE | (reg & 0x1F));
    // Send the mask to set the bits
    spi_transfer_byte(mask);
    enc28j60_chip_select_high();
}

// Clear bits in a control register
void enc28j60_bit_field_clear(uint8_t reg, uint8_t mask) {
    enc28j60_chip_select_low();
    // Send the Bit Field Clear opcode (BFC) with the 5-bit register address
    spi_transfer_byte(BFC_OPCODE | (reg & 0x1F));
    // Send the mask to clear the bits
    spi_transfer_byte(mask);
    enc28j60_chip_select_high();
}

// Function to send a packet to the ENC28J60
void enc28j60_send_packet(const uint8_t* data, uint16_t len) {
    // A full driver would need to check for available buffer space first.
    
    // Write the data to the transmit buffer.
    enc28j60_chip_select_low();
    // Send the Write Buffer Memory (WBM) instruction
    spi_transfer_byte(WBM_OPCODE);
    // Send the actual data payload
    spi_send_data((uint8_t*)data, len);
    enc28j60_chip_select_high();

    // Set the TXRTS bit in ECON1 to initiate the transmission.
    enc28j60_bit_field_set(ECON1_REG, ECON1_TXRTS);
}

// Initial placeholder for ENC28J60 init
void enc28j60_init(void) {
    // A full driver would include a proper initialization sequence here,
    // including resetting the chip, configuring buffers, and setting up
    // the MAC and PHY. This is a basic placeholder.
}

