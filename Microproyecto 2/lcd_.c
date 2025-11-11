#include <xc.h>
#include <stdint.h>

// Configuración del PIC18F4550
#pragma config FOSC = INTOSCIO_EC  // Oscilador interno, RA6 como I/O
#pragma config WDT = OFF           // Watchdog desactivado
#pragma config LVP = OFF           // Low Voltage Programming OFF
#pragma config PBADEN = OFF        // PORTB digital al inicio
#pragma config MCLRE = OFF          // MCLR deshabilitado
#pragma config PWRT = ON           // Power-up Timer habilitado
#pragma config BOR = ON            // Brown-out Reset habilitado

#define _XTAL_FREQ 8000000  // Frecuencia de 8MHz

// Definición de pines del LCD (modo 4 bits) - Usando PORTB
#define RS PORTBbits.RB0    // Register Select (Pin 4 del LCD)
#define EN PORTBbits.RB1    // Enable (Pin 6 del LCD)
#define D4 PORTBbits.RB4    // Data bit 4 (Pin 11 del LCD)
#define D5 PORTBbits.RB5    // Data bit 5 (Pin 12 del LCD)
#define D6 PORTBbits.RB6    // Data bit 6 (Pin 13 del LCD)
#define D7 PORTBbits.RB7    // Data bit 7 (Pin 14 del LCD)

// Prototipos de funciones
void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_Write_String(const char *str);
void LCD_Clear(void);
void LCD_Set_Cursor(uint8_t row, uint8_t col);

// Función para enviar nibble (4 bits)
void LCD_Send_Nibble(uint8_t nibble) {
    D4 = (nibble >> 0) & 0x01;
    D5 = (nibble >> 1) & 0x01;
    D6 = (nibble >> 2) & 0x01;
    D7 = (nibble >> 3) & 0x01;
    
    EN = 1;
    __delay_us(1);
    EN = 0;
    __delay_us(100);
}

// Enviar comando al LCD
void LCD_Command(uint8_t cmd) {
    RS = 0;  // Modo comando
    LCD_Send_Nibble(cmd >> 4);   // Enviar los 4 bits altos
    LCD_Send_Nibble(cmd & 0x0F); // Enviar los 4 bits bajos
    __delay_ms(2);
}

// Enviar dato al LCD
void LCD_Data(uint8_t data) {
    RS = 1;  // Modo dato
    LCD_Send_Nibble(data >> 4);   // Enviar los 4 bits altos
    LCD_Send_Nibble(data & 0x0F); // Enviar los 4 bits bajos
    __delay_ms(2);
}

// Inicializar LCD en modo 4 bits
void LCD_Init(void) {
    __delay_ms(50);  // Esperar estabilización al encender
    
    RS = 0;
    EN = 0;
    
    // Secuencia de inicialización para modo 4 bits
    LCD_Send_Nibble(0x03);
    __delay_ms(5);
    LCD_Send_Nibble(0x03);
    __delay_us(150);
    LCD_Send_Nibble(0x03);
    __delay_ms(1);
    LCD_Send_Nibble(0x02);  // Cambiar a modo 4 bits
    
    // Configuración del LCD 16x2
    LCD_Command(0x28);  // 4 bits, 2 líneas, 5x7 puntos
    LCD_Command(0x0C);  // Display ON, cursor OFF, blink OFF
    LCD_Command(0x06);  // Incrementar cursor, no desplazar display
    LCD_Command(0x01);  // Limpiar display
    __delay_ms(2);
}

// Limpiar pantalla
void LCD_Clear(void) {
    LCD_Command(0x01);
    __delay_ms(2);
}

// Posicionar cursor en LCD 16x2
void LCD_Set_Cursor(uint8_t row, uint8_t col) {
    uint8_t addr;
    
    if(row == 1)
        addr = 0x80 + col;  // Primera línea (0x00-0x0F)
    else
        addr = 0xC0 + col;  // Segunda línea (0x40-0x4F)
    
    LCD_Command(addr);
}

// Escribir cadena de texto
void LCD_Write_String(const char *str) {
    while(*str) {
        LCD_Data(*str++);
    }
}

// Función principal
void main(void) {
    // Configurar oscilador interno a 8MHz
    OSCCON = 0x72;  // 8MHz, oscilador interno estable
    while(!OSCCONbits.IOFS);  // Esperar a que el oscilador sea estable
    
    // Configurar puertos
    ADCON1 = 0x0F;  // Todos los pines digitales
    CMCON = 0x07;   // Comparadores apagados
    
    // Configurar PORTB
    TRISB = 0x00;  // PORTB como salida
    PORTB = 0x00;  // Limpiar PORTB
    
    // Inicializar LCD
    LCD_Init();
    LCD_Clear();  // Limpiar la pantalla (elimina el warning)
    
    // Mostrar "Hola Mundo" centrado
    LCD_Set_Cursor(1, 2);  // Fila 1, columna 2 (centrado)
    LCD_Write_String("Hola Mundo!");
    
    LCD_Set_Cursor(2, 3);  // Fila 2, columna 3 (centrado)
    LCD_Write_String("PIC18F4550");
    
    // Bucle infinito
    while(1) {
        // Aquí puedes agregar más código
        // Ejemplo: para limpiar y actualizar la pantalla cada 5 segundos
        // __delay_ms(5000);
        // LCD_Clear();
        // LCD_Set_Cursor(1, 0);
        // LCD_Write_String("Actualizado!");
    }
    
    return;
}