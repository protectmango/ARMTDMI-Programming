#include"header.h"
extern u32 flag;
extern CAN1 m1;
void CAN1_RX_Handler(void) __irq
{
	m1.id=C1RID;
	m1.dlc=(C1RFS>>16)&0xf;
	m1.rtr=(C1RFS>>30)&1;
	if(m1.rtr==0)
	{
		m1.byteA=C1RDA;
		m1.byteB=C1RDB;
	}
	C1CMR=(1<<2);
	flag=1;
	VICVectAddr=0;
}
void en_can1_interrupt(void)
{
	C1IER=1;
}
void config_vic_for_CAN1(void)
{
	VICIntSelect=0;
	VICVectCntl2=26|(1<<5);
	VICVectAddr2=(u32) CAN1_RX_Handler;
	VICIntEnable=(1<<26);
}

