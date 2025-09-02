#include"header.h"
void can2_init(void)
{
	VPBDIV=1;
	PINSEL1|=0x14000;
	C2MOD=1;
	AFMR=0;
	C2BTR=0X001C001D;
	C2MOD=0;
}
void can2_tx(CAN2 v)
{
	C2TID1=v.id;
	C2TFI1=(v.dlc<<16);
	if(v.rtr==0)
	{
		C2TDA1=v.byteA;
		C2TDB1=v.byteB;
	}
	else
		C2TFI1|=(1<<30);

	C2CMR=1|(1<<5);
	while(TCS2==0);
}
void can2_rx(CAN2 *ptr)
{
	while(RBS2==0);
	ptr->id=C2RID;
	ptr->dlc=(C2RFS>>16)&0xf;
	ptr->rtr=(C2RFS>>30)&1;
	if(ptr->rtr==0)
	{
		ptr->byteA=C2RDA;
		ptr->byteB=C2RDB;
	}
	C2CMR=(1<<2);
}

