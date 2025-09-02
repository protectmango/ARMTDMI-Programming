#include"header.h"
void can1_init(void)
{
	VPBDIV=1;
	PINSEL1|=0x40000;
	C1MOD=1;
	AFMR=2;
	C1BTR=0X001C001D;
	C1MOD=0;
}
void can1_tx(CAN1 v)
{
	C1TID1=v.id;
	C1TFI1=(v.dlc<<16);
	if(v.rtr==0)
	{
		C1TDA1=v.byteA;
		C1TDB1=v.byteB;
	}
	else
		C1TFI1|=(1<<30);

	C1CMR=1|(1<<5);
	while(TCS==0);
}
void can1_rx(CAN1 *ptr)
{
	while(RBS==0);
	ptr->id=C1RID;
	ptr->dlc=(C1RFS>>16)&0xf;
	ptr->rtr=(C1RFS>>30)&1;
	if(ptr->rtr==0)
	{
		ptr->byteA=C1RDA;
		ptr->byteB=C1RDB;
	}
	C1CMR=(1<<2);
}

