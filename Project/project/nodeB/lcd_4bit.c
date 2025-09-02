#include<lpc21xx.h>
#include "header.h"

main()
{
lcd_init();
lcd_cgram();
lcd_cmd(0x80);
lcd_data(0);
	lcd_cmd(0x87);
lcd_data(1);
	lcd_cmd(0x8f);
lcd_data(2);
	lcd_cmd(0xc0);
	lcd_string("Te:");
	lcd_cmd(0xca);
	lcd_string("Sp:");
}

