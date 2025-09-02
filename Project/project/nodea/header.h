
#include<lpc21xx.h>
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short int u16;

extern void delay_sec(unsigned int sec);
extern void delay_ms(unsigned int ms);

#define DONE ((ADDR>>31)&1)

extern void adc_init(void);
extern u32 adc_read(u8 ch_num);

extern void config_vic_for_eint1(void);
extern void config_eint1(void);

extern void config_vic_for_eint0(void);
extern void config_eint0(void);

extern void config_vic_for_eint2(void);
extern void config_eint2(void);

extern void config_vic_for_timer1(void);
extern void en_timer1_interrupt(void);

#define TCS2 (C2GSR&8)
#define RBS2 (C2GSR&1)

typedef struct CAN2_MSG
{
u32 id;
u32 rtr;
u32 dlc;
u32 byteA;
u32 byteB;
}CAN2;


extern void can2_init(void);
extern void can2_tx(CAN2 v);
extern void can2_rx(CAN2 *ptr);

