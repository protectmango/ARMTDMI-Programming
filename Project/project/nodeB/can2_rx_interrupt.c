#include"header.h"
extern u32 flag;
extern CAN2 m1;
void CAN2_RX_Handler(void) __irq
{
	m1.id=C2RID;
	m1.dlc=(C2RFS>>16)&0xf;
	m1.rtr=(C2RFS>>30)&1;
	if(m1.rtr==0)
	{
		m1.byteA=C2RDA;
		m1.byteB=C2RDB;
	}
	C2CMR=(1<<2);
	flag=1;
	VICVectAddr=0;
}
void en_can2_interrupt(void)
{
	C2IER=1;
}
void config_vic_for_CAN2(void)
{
	VICIntSelect=0;
	VICVectCntl2=27|(1<<5);
	VICVectAddr2=(u32) CAN2_RX_Handler;
	VICIntEnable=(1<<27);
}

