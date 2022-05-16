#include "lcd.h"
#define RS PORTC1
#define EN PORTC0
#define FirstLine 0x80
#define SecondLine 0xc0

void Clear_LCD()
{
	WriteCommand(0x01);
	//LCD_Command (0x01);		/* Clear display */
	//_delay_ms(2);
	WriteCommand (0x80);		/* Cursor at home position */
}
void WriteCommand(char comm)
{
	DDRD = 0xff;//BITDATA
	DDRC = 0xff;//EN,RS
	
	PORTC &= ~ (1<<RS);	
	PORTD = (PORTD & 0x0F) | (comm & 0xF0);
		
	PORTC|= (1<<EN);		
	_delay_ms(1);
	PORTC &= ~ (1<<EN);
	_delay_ms(2);
	PORTD = (PORTD & 0x0F) |( (comm << 4));
	PORTC |= (1<<EN);
	_delay_ms(2);
	PORTC&= ~ (1<<EN);
	_delay_ms(70);
	
}

void WriteDataChar(char data)
{
	DDRD = 0xf0;
	DDRC = 0x03;
	PORTC|= (1<<RS);
	PORTD = (PORTD & 0x0F) | (data & 0xF0);
			
	PORTC |= (1<<EN);		
	_delay_ms(1);
	PORTC &= ~ (1<<EN);
	_delay_ms(1);
	PORTD = (PORTD & 0x0F) | ((data << 4));
	PORTC |= (1<<EN);
	_delay_ms(1);
	PORTC &= ~ (1<<EN);
	_delay_ms(2);
	
}
void WriteDataString(char *string_of_characters)
{
	while(*string_of_characters > 0)
	{
		WriteDataChar(*string_of_characters++);
	}
}
void LCD_Init()
{
	WriteCommand(0x33);
	WriteCommand(0x32);	
	WriteCommand(0x28);	
	WriteCommand(0x0c);	
	WriteCommand(0x06);	
	WriteCommand(0x01);
}