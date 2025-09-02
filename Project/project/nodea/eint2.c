#include"header.h"
extern u8 flag3;
void EINT2_Handler(void)__irq
{	

  	flag3=1;
	
	EXTINT|=4;
	VICVectAddr=0;


}
void config_vic_for_eint2(void)
{

	//VICIntSelect=0;
	VICVectCntl3=16|(1<<5);
	VICVectAddr3=(u32)EINT2_Handler;
	VICIntEnable|=(1<<16);
}
void config_eint2(void)
{
	EXTPOLAR=0;
	EXTMODE|=4;
	PINSEL0|=0x80000000;
}

