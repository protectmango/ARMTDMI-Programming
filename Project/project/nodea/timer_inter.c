#include"header.h"
int a[]={15,60,30,15,15};
unsigned int pclk;
extern u8 flag;
void TIMER1_Handler(void) __irq
{	
	
	flag=1;
	T1IR=1;
	VICVectAddr=0;
}
void config_vic_for_timer1(void)
{
	VICIntSelect=0;
	VICVectCntl2=5|(1<<5);
	VICVectAddr2=(int)TIMER1_Handler;
	VICIntEnable|=(1<<5);
}
void en_timer1_interrupt(void)
{
	T1MCR=3;
	pclk=a[VPBDIV]*1000000;
	T1PC=0;
	T1PR=pclk-1;
	T1TC=0;
	T1MR0=1;
	T1TCR=1;
}

