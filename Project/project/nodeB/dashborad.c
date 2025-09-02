#include"header.h"
#define HL (1<<18)
#define LI (1<<17)
#define RI (1<<19)
CAN2 m1;
u32 flag;
main()
{
	u8 f1,f2;
	u32 speed,per;
	u32 eng_val;
	float temp,vol;

	f1=f2=0;
	can2_init();
	config_vic_for_CAN2();
	en_can2_interrupt();
	lcd_init();
	lcd_cgram();
	IODIR0=HL|LI|RI;
	IOSET0=HL|LI|RI;
	lcd_string("ECU 2");
	
	
	while(1)
	{	

	 	l:

	 	if(flag)

		{
				flag=0;
				if(m1.id==0x215)
				{

					//speed
					speed=m1.byteA;
					lcd_cmd(0xc0);
					lcd_string("S:");
					per=((float)speed/1023)*100;
					//lcd_cmd(0xc1);
				//	lcd_string("  ");
					lcd_cmd(0xc2);
					lcd_integer(per);
					lcd_string("Kmph");
					
				}

				if(m1.id==0x213)  //headlight

				{

					if((m1.byteA&0xf)==0x4)

					{
											 
						lcd_cmd(0x87);
						lcd_data(1);
				    	IOCLR0|=HL;
					
					}

					if((m1.byteA&0xf)==0x5)

					{
						
						lcd_cmd(0x87);
						lcd_data(' ');
				     	IOSET0|=HL;
					

					}

				}
				if(m1.id==0x212)   //leftindicator

				{

					if((m1.byteA&0xf)==0x2)

					{
						lcd_cmd(0x81);
						lcd_data(' ');

						f1=0;

						f2=1;
					 
					}

					if((m1.byteA&0xf)==0x3)

					{
						lcd_cmd(0x81);
						lcd_data(' ');
						f2=0;
					//	uart0_tx_string("k\r\n");
					}

				}
				if(m1.id==0x214) //right indi

				{

					if((m1.byteA&0xf)==0x6)

					{
						lcd_cmd(0x8c);
						lcd_data(' ');

						f1=1;
						f2=0;
					//	uart0_tx_string("l\r\n");
					}

					if((m1.byteA&0xf)==0x7)

					{
						lcd_cmd(0x8c);
						lcd_data(' ');
						f1=0;
					}

				}
				if(m1.id==0x216)

				{

					//Engine temperature
					eng_val=m1.byteA;
					vol=(eng_val*3.3)/1023;
					temp=(vol-0.5)/0.01;
					lcd_cmd(0xc9);
					lcd_string("T:");
					lcd_cmd(0xcb);
				//	lcd_string("temp: ");
					lcd_float_adc(temp);
					lcd_string("^C");
			
				}

		}

		if(f1)

		{

			while(flag==0)

			{
				
				lcd_cmd(0x80);
				lcd_data(0);
				IOCLR0|=LI;
				delay_ms(250);

				lcd_cmd(0x80);
				lcd_data(' ');
				IOSET0|=LI;
				delay_ms(250);
			}

			goto l;

		}

		if(f2)

		{

			while(flag==0)

			{
			
				lcd_cmd(0x8f);
				lcd_data(2);
			IOCLR0|=RI;
				delay_ms(250);

				lcd_cmd(0x8f);
				lcd_data(' ');
				IOSET0|=RI;
				delay_ms(250);
			}

			goto l;

		}
		//delay_ms(250);		
  
	}
	
}

