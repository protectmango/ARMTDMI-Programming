#include"header.h"
void delay_sec(unsigned int sec)
{
	int a[]={15,60,30,15,15};
	unsigned int pclk;
	pclk=a[VPBDIV]*1000000;
	T0PC=0;
	T0PR=pclk-1;
	T0TC=0;
	T0TCR=1;	//START TIMER 0
	while(T0TC<sec);
	T0TCR=0;	//STOP TIMER 0
}
void delay_ms(unsigned int ms)
{
	int a[]={15,60,30,15,15};
	unsigned int pclk;
	pclk=a[VPBDIV]*1000;
	T0PC=0;
	T0PR=pclk-1;
	T0TC=0;
	T0TCR=1;	//start timer 0
	while(T0TC<ms);
	T0TCR=0;	//stop timer 0
}

