																		 #include"header.h"
//#define SW1 ((IOPIN0>>14)&1)
//#define SW2 ((IOPIN0>>15)&1)
//#define SW3 ((IOPIN0>>16)&1)

u8 flag,flag1,flag2,flag3;
CAN2 v1,v2;
main()
{
	u8 f1,f2,f3;
	u32 eng_temp,speed;
	f1=f2=f3=0;
	can2_init();
	adc_init();
	config_vic_for_timer1();
	en_timer1_interrupt();
	config_vic_for_eint0();
	config_eint0();
	config_vic_for_eint1();
	config_eint1();
	config_vic_for_eint2();
	config_eint2();
	v1.dlc=1;
	v1.rtr=0;

	v1.byteA=0;

	v1.byteB=0;
	v2.dlc=2;
	v2.rtr=0;

	v2.byteA=0;

	v2.byteB=0;
	while(1)
	{
		if(flag3)
		{
			flag3=0;
	
			v1.id=0x513;
			f1^=1;
			if(f1)
			{
				v1.byteA=0x2;
			}
			else
			{
				v1.byteA=0x3;
			}
			can2_tx(v1);
		}
		if(flag2)
		{
			flag2=0;
			v1.id=0x515;
			f2^=1;
			if(f2)
			{
				v1.byteA=0x6;
			}
			else
			{
				v1.byteA=0x7;
			}
			can2_tx(v1);
		}
		if(flag1)
		{
			flag1=0;
			v1.id=0x514;
			f3^=1;
			if(f3)
			{
				v1.byteA=0x4;
			}
			else
			{
				v1.byteA=0x5;
			}
			can2_tx(v1);
		}
		if(flag)
		{
			flag=0;
			v2.id=0x512;
			speed=adc_read(2);
			v2.byteA=speed;
			can2_tx(v2);
			delay_ms(5);
			v2.id=0x516;
			eng_temp=adc_read(1);
			v2.byteA=eng_temp;
			can2_tx(v2);
		}
	}
}
