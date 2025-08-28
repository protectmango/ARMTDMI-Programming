// timer_driver.c
#include "header.h"
#include <stdio.h>
#include <string.h>

// Dummy function to simulate a sensor reading
float read_temperature_sensor(void) {
    return 25.5;
}

// Dummy function to simulate an RTC time
uint32_t read_rtc_time(void) {
    return 1672531200;
}

// Function to log and send data, called by the Timer ISR
void log_data_and_send(void) {
    float temperature = read_temperature_sensor();
    uint32_t timestamp = read_rtc_time();
    char log_string[100];
    
    // Create a JSON-like string for the data payload
    sprintf(log_string, "{\"timestamp\":%lu,\"temperature\":%.2f}", timestamp, temperature);
    
    // In a real application, you would call the ENC28J60 send function here.
    enc28j60_send_packet((uint8_t*)log_string, strlen(log_string));
}

// Initialize Timer 1
void timer1_init(void) {
    // Set VPBDIV to 0x01 to make PCLK=CCLK/4
    VPBDIV = 0x01; 

    // Set prescale register to get a 1us tick
    T1PR = PCLK / 1000000 - 1;
    
    // Set match register for 1 second (1000ms)
    T1MR0 = TIMER_INTERVAL_MS * 1000;

    // Configure Match Control Register (MCR)
    T1MCR |= (1 << 0) | (1 << 1); // Interrupt on Match 0, Reset on Match 0
    
    // Reset and enable Timer 1
    T1TCR = (1 << 1);
    T1TCR = (1 << 0);

    // VIC setup for Timer 1
    VICIntSelect &= ~(1 << 5); // Set as IRQ
    VICVectCntl5 = (1 << 5) | 5; // Enable, set slot to 5, set source to 5 (Timer 1)
    VICVectAddr5 = (uint32_t)Timer1_ISR; // Set ISR address
    VICIntEnable = (1 << 5); // Enable Timer 1 interrupt
}

// Timer 1 Interrupt Service Routine
__irq void Timer1_ISR(void) {
    log_data_and_send();
    
    T1IR |= (1 << 0); // Clear Match 0 interrupt flag
    VICVectAddr = 0; // Clear interrupt from VIC
}

