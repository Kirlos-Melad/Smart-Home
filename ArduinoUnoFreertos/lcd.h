
#ifndef LCD_H_
#define LCD_H_



#include <avr/io.h>
#include <avr/delay.h>
void WriteCommand(char comm);
void WriteDataChar(char data);
void WriteDataString(char *string_of_characters);
void LCD_Init();
void Clear_LCD();


#endif /* LCD_H_ */