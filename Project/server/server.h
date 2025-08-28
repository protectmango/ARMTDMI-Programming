// server.h
#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

// Function prototypes for the web server application
uint16_t enc28j60_receive_packet(uint8_t* buffer, uint16_t buffer_size);
void process_packet(uint8_t* buffer, uint16_t len);
void server_loop(void);

#endif // SERVER_H

