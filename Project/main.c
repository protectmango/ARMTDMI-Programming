// main.c
#include "header.h"

int main(void) {
    // Initialize PLL and VIC
    pll_init();
    vic_init();

    // Initialize SPI for ENC28J60 communication
    spi_init();
    
    // Initialize ENC28J60 (requires a full library)
    enc28j60_init();

    // Initialize the timer for data logging
    timer1_init();

    while (1) {
        // Main loop for non-interrupt driven tasks
        // In this example, the data logging happens in the timer interrupt.
        // You can add other tasks here that don't need to be time-critical.
    }
}

