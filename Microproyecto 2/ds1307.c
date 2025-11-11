#include "i2c.h"
#include "LCD.h"
#include <stdio.h>
#define DS1307_ADDR 0b11010000

unsigned char BCD2DEC(unsigned char val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

unsigned char DEC2BCD(unsigned char val) {
    return ((val / 10) << 4) | (val % 10);
}

void RTC_Init(void) {
    unsigned char sec;

    I2C_Master_Start();
    I2C_Master_Write(DS1307_ADDR);
    I2C_Master_Write(0x00);
    I2C_Master_RepeatedStart();
    I2C_Master_Write(DS1307_ADDR | 1);
    sec = I2C_Master_Read(0);
    I2C_Master_Stop();

    if (sec & 0x80) {
        // Mostrar mensaje si el reloj estaba detenido
        LCD_Clear();
        LCD_String_xy(0, 0, "RTC detenido");
        __delay_ms(1000);

        I2C_Master_Start();
        I2C_Master_Write(DS1307_ADDR);
        I2C_Master_Write(0x00);
        I2C_Master_Write(sec & 0x7F); // Activa el reloj (CH=0)
        I2C_Master_Stop();
    }
}

void RTC_SetDateTime(unsigned char h, unsigned char m, unsigned char s,
                     unsigned char d, unsigned char mo, unsigned char y) {
    unsigned char check;

    // Forzar CH = 0 en segundos
    s &= 0x7F;

    // Iniciar comunicación I2C
    I2C_Master_Start();
    I2C_Master_Write(DS1307_ADDR);  // Dirección del DS1307 para escritura
    I2C_Master_Write(0b00000000);         // Registro de segundos

    // Enviar datos en formato BCD
    I2C_Master_Write(DEC2BCD(s));   // Segundos (CH=0)
    I2C_Master_Write(DEC2BCD(m));   // Minutos
    I2C_Master_Write(DEC2BCD(h));   // Horas
    I2C_Master_Write(DEC2BCD(1));   // Día de la semana (fijo en 1)
    I2C_Master_Write(DEC2BCD(d));   // Día
    I2C_Master_Write(DEC2BCD(mo));  // Mes
    I2C_Master_Write(DEC2BCD(y));   // Año
    I2C_Master_Stop();
}


void RTC_GetDateTime(unsigned char *hour, unsigned char *min, unsigned char *sec,
                     unsigned char *day, unsigned char *month, unsigned char *year) {
    I2C_Master_Start();
    I2C_Master_Write(DS1307_ADDR);
    I2C_Master_Write(0b00000000);
    I2C_Master_RepeatedStart();
    I2C_Master_Write(DS1307_ADDR | 1);

    *sec   = BCD2DEC(I2C_Master_Read(1));
    *min   = BCD2DEC(I2C_Master_Read(1));
    *hour  = BCD2DEC(I2C_Master_Read(1));
    I2C_Master_Read(1); // Día de la semana (omitido)
    *day   = BCD2DEC(I2C_Master_Read(1));
    *month = BCD2DEC(I2C_Master_Read(1));
    *year  = BCD2DEC(I2C_Master_Read(0));
    I2C_Master_Stop();
}
