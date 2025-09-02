#include"header.h"
extern u8 flag1;
void EINT0_Handler(void)__irq
{
	flag1=1;	
	

	EXTINT|=1;
	VICVectAddr=0;

}
void config_vic_for_eint0(void)
{
	
	VICIntSelect=0;
	VICVectCntl1=14|(1<<5);
	VICVectAddr1=(u32)EINT0_Handler;
	VICIntEnable|=(1<<14);
}
void config_eint0(void)
{
	EXTPOLAR=0;
	EXTMODE|=1;
	PINSEL1|=1;
}

