#include "config.h"
#include "lcd.h"
#include <stdio.h>

void LCD_Pulse(void) {
    LCD_EN = 1;
    __delay_us(1);
    LCD_EN = 0;
    __delay_us(100);
}

void LCD_SendNibble(unsigned char nibble) {
    LCD_D4 = (nibble >> 0) & 0x01;
    LCD_D5 = (nibble >> 1) & 0x01;
    LCD_D6 = (nibble >> 2) & 0x01;
    LCD_D7 = (nibble >> 3) & 0x01;
    LCD_Pulse();
}

void LCD_Init(void) {
    // Configurar pines LCD como salidas - CORREGIDO
    TRISAbits.TRISA1 = 0;  // RS (cambio de RB0 a RA1)
    TRISAbits.TRISA2 = 0;  // EN (cambio de RB1 a RA2)
    TRISBbits.TRISB4 = 0;  // D4
    TRISBbits.TRISB5 = 0;  // D5
    TRISBbits.TRISB6 = 0;  // D6
    TRISBbits.TRISB7 = 0;  // D7
    
    __delay_ms(50);
    LCD_RS = 0;
    
    LCD_SendNibble(0x03);
    __delay_ms(5);
    LCD_SendNibble(0x03);
    __delay_us(150);
    LCD_SendNibble(0x03);
    LCD_SendNibble(0x02);
    
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Clear();
}

void LCD_Command(unsigned char cmd) {
    LCD_RS = 0;
    LCD_SendNibble(cmd >> 4);
    LCD_SendNibble(cmd & 0x0F);
    __delay_ms(2);
}

void LCD_Char(unsigned char data) {
    LCD_RS = 1;
    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);
    __delay_ms(1);
}

void LCD_String(const char *str) {
    while(*str) {
        LCD_Char(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Command(LCD_CLEAR);
    __delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    unsigned char position;
    if(row == 1) {
        position = LCD_LINE1 + col;
    } else {
        position = LCD_LINE2 + col;
    }
    LCD_Command(position);
}

void LCD_Float(float num, unsigned char decimales) {
    char buffer[16];
    sprintf(buffer, "%.*f", decimales, num);
    LCD_String(buffer);
}

void LCD_String_xy(char row, char pos, const char *str) {
    char location = 0;
    if(row < 1) {
        location = (0x80) | ((pos) & 0x0f);
        LCD_Command(location);
    } else {
        location = (0xC0) | ((pos) & 0x0f);
        LCD_Command(location);    
    }  
    
    LCD_String(str);
}