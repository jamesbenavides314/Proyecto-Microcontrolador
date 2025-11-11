#ifndef LCD_H
#define LCD_H

// Definición de pines del LCD - CORREGIDO
#define LCD_RS PORTAbits.RA1  // ? CAMBIO: Movido de RB0 a RA1
#define LCD_EN PORTAbits.RA2  // ? CAMBIO: Movido de RB1 a RA2
#define LCD_D4 PORTBbits.RB4
#define LCD_D5 PORTBbits.RB5
#define LCD_D6 PORTBbits.RB6
#define LCD_D7 PORTBbits.RB7

#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_LINE1 0x80
#define LCD_LINE2 0xC0

void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char data);
void LCD_String(const char *str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Float(float num, unsigned char decimales);
void LCD_String_xy(char row, char pos, const char *str);

#endif