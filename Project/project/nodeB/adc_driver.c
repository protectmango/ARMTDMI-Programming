#include"header.h"
void adc_init(void)
{
	PINSEL1|=0x15400000;
	ADCR=0x00200400;
}
u32 adc_read(u8 ch_num)
{
	u32 result=0;
	ADCR|=(1<<ch_num);
	ADCR|=(1<<24);
	while(DONE==0);
	ADCR^=(1<<24);
	ADCR^=(1<<ch_num);
	result=(ADDR>>6)&0x3ff;
	return result;
}
