#include"header.h"
extern u8 flag2;
void EINT1_Handler(void)__irq
{
	flag2=1;	

	EXTINT|=2;
	VICVectAddr=0;


}
void config_vic_for_eint1(void)
{
	VICIntSelect=0;
	VICVectCntl0=15|(1<<5);
	VICVectAddr0=(u32)EINT1_Handler;
	VICIntEnable|=(1<<15);
}
void config_eint1(void)
{
	EXTPOLAR=0;
	EXTMODE|=2;
	PINSEL0|=0x20000000;
}

