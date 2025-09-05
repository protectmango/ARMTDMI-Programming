#include <LPC21xx.h>
#include <stdio.h>
#include <string.h>

#define LED1 (1<<17)
#define LED2 (1<<18)
#define LED3 (1<<19)

#define RS (1<<17)
#define RW (1<<18)
#define EN (1<<19)
#define LCD_DATA (0x00F00000)

volatile char rxData[32];
volatile int idx = 0;

// === Delay ===
void delay(unsigned int t) {
    unsigned int i;
    for(i=0; i<t*1000; i++);
}
// === LCD ===
void LCD_Command(unsigned char cmd) {
    IO1CLR = RS|RW|LCD_DATA;
    IO1SET = (cmd & 0xF0) << 16;
    IO1SET = EN; delay(1); IO1CLR = EN;
    IO1CLR = LCD_DATA;
    IO1SET = (cmd & 0x0F) << 20;
    IO1SET = EN; delay(1); IO1CLR = EN;
    delay(2);
}

void LCD_Char(unsigned char data) {
    IO1SET = RS;
    IO1CLR = RW|LCD_DATA;
    IO1SET = (data & 0xF0) << 16;
    IO1SET = EN; delay(1); IO1CLR = EN;
    IO1CLR = LCD_DATA;
    IO1SET = (data & 0x0F) << 20;
    IO1SET = EN; delay(1); IO1CLR = EN;
    delay(2);
}

void LCD_String(char *str) {
    while(*str) LCD_Char(*str++);
}

void LCD_Init(void) {
    IODIR1 |= RS | RW | EN | LCD_DATA;
    delay(20);
    LCD_Command(0x28); // 4-bit, 2 line
    LCD_Command(0x0C); // Display on
    LCD_Command(0x06); // Entry mode
    LCD_Command(0x01); // Clear
}


// === UART0 ===
void UART0_TxChar(char c) {
    while(!(U0LSR & 0x20));
    U0THR = c;
}

void UART0_SendString(char *str) {
    while(*str) UART0_TxChar(*str++);
}

__irq void UART0_ISR(void) {
    char c;
    c = U0RBR;

    // Active-Low LED logic
    if(c=='1') IOCLR0=LED1;   // LED1 ON
    else if(c=='2') IOCLR0=LED2; // LED2 ON
    else if(c=='3') IOCLR0=LED3; // LED3 ON
    else if(c=='4') IOSET0=LED1; // LED1 OFF
    else if(c=='5') IOSET0=LED2; // LED2 OFF
    else if(c=='6') IOSET0=LED3; // LED3 OFF
    else if(c=='#') {
        rxData[idx]='\0';
        LCD_Command(0x01);
        LCD_String((char*)rxData);
        idx=0;
    }
    else {
        rxData[idx++] = c;
    }

    VICVectAddr = 0;
}



void UART0_Init(void) {
    PINSEL0 |= 0x00000005;      // P0.0=TXD0, P0.1=RXD0
    U0LCR = 0x83;               // 8-bit, 1 stop, no parity, DLAB=1
    U0DLL = 97;                 // Baud = 9600 @ 15MHz
    U0DLM = 0;
    U0LCR = 0x03;               // DLAB=0

    U0IER = 0x01;               // Enable RDA interrupt
    VICVectAddr0 = (unsigned long)UART0_ISR;
    VICVectCntl0 = 0x26;        // UART0, slot 0
    VICIntEnable = 1<<6;        // Enable UART0 interrupt
}


// === ADC ===
void ADC_Init(void) {
    PINSEL1 |= (1<<24);  // P0.28 = AD1
    PINSEL1 |= (1<<26);  // P0.29 = AD2
    ADCR = (1<<1)|(1<<2)|(4<<8)|(1<<16)|(1<<21); // Enable ADC, CLKDIV, PDN
}

unsigned int ADC_Read(unsigned char ch) {
    unsigned int result;
    ADCR &= 0xFFFFFF00;          // Clear channel selection
    ADCR |= (1<<ch) | (1<<24);   // Select channel, start conversion
    while((ADDR & (1U<<31)) == 0); // Wait DONE bit
    result = (ADDR >> 6) & 0x3FF;
    return result;
}

float Temp_Celsius(unsigned int adc) {
    float voltage, tempC;
    voltage = (adc * 3.3f) / 1024.0f;   // in Volts
    tempC = ((voltage * 1000.0f) - 500.0f) / 10.0f;
    return tempC;
}


// === MAIN ===
int main(void) {
    unsigned int pot, rawTemp;
    float tempC;
    char buffer[32];

    IODIR0 |= LED1|LED2|LED3;
    UART0_Init();
    LCD_Init();
    ADC_Init();

    while(1) {
    pot    = ADC_Read(1);
rawTemp= ADC_Read(2);
tempC  = Temp_Celsius(rawTemp);

/* one clean line, once every 5 s */
sprintf(buffer, "Temp:%.2fC Speed:%u\n", tempC, pot);
UART0_SendString(buffer);

delay(5000);   // <-- 5 second delay
		}

}
