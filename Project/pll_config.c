// pll_config.c
#include "header.h"

// Function to initialize PLL and configure clocks
void pll_init(void) {
    // Configure and enable PLL
    PLLCFG = (1 << 5) | 4; // M=5, P=2 (CCLK = 60MHz)
    PLLCON = 1;
    PLLFEED = 0xAA;
    PLLFEED = 0x55;
    while (!(PLLSTAT & (1 << 10))); // Wait for PLL lock
    PLLCON = 3; // Connect PLL
    PLLFEED = 0xAA;
    PLLFEED = 0x55;
    VPBDIV = 0x01; // PCLK = CCLK/4
}

// Function to initialize Vectored Interrupt Controller (VIC)
void vic_init(void) {
    // VIC initialization is handled in each peripheral driver
}

