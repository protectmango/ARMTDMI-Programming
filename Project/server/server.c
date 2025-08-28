// server.c
#include "header.h"
#include <stdio.h>
#include <string.h>

// Dummy function to simulate receiving a packet
uint16_t enc28j60_receive_packet(uint8_t* buffer, uint16_t buffer_size) {
    // This is a placeholder. A real implementation would read from the
    // ENC28J60's receive buffer and return the length of the received packet.
    return 0; // Return 0 to indicate no packet received
}

// Function to handle a received packet
void process_packet(uint8_t* buffer, uint16_t len) {
    // In a real application, you would parse the packet here.
    // This could involve checking for HTTP GET requests,
    // extracting data, and then sending a response.
    
    // For this example, we'll just print a simple message.
    // Note: A printf on an embedded system requires a UART or other
    // debug interface to be set up.
    printf("Received packet of length %d\n", len);
}

// The main server loop
void server_loop(void) {
    uint8_t rx_buffer[100]; // Small buffer for testing
    uint16_t packet_len;

    while (1) {
        // Poll for incoming packets
        packet_len = enc28j60_receive_packet(rx_buffer, sizeof(rx_buffer));

        if (packet_len > 0) {
            // Process the received packet if one exists
            process_packet(rx_buffer, packet_len);
        }
    }
}

// Main function
int main(void) {
    // Initialize PLL and VIC
    pll_init();
    vic_init();

    // Initialize SPI for ENC28J60 communication
    spi_init();
    
    // Initialize ENC28J60
    enc28j60_init();

    // Start the server loop
    server_loop();
    
    return 0;
}

