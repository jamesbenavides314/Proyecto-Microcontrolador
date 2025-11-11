#include <xc.h>
#include <stdio.h>
#include "LCD.h"
#include "Config.h"
#include "i2c.h"
#include "ds1307.h"

void main(void) {
    unsigned char h, m, s, d, mo, y;
    char buffer[17];

    OSCCONbits.IRCF = 0b111; // 8 MHz
    OSCCONbits.SCS = 0b10;
    ADCON1 = 0x0F;

    LCD_Init();
    LCD_Clear();
    I2C_Master_Init(100000); // MSSP I2C en RB0/RB1
    LCD_String_xy(0, 0, "Configurando RTC");
    RTC_Init();
    RTC_SetDateTime(14, 30, 0, 8, 4, 25); // Configuración inicial
    LCD_Clear();

    while (1) {        
        RTC_GetDateTime(&h, &m, &s, &d, &mo, &y);
        
        sprintf(buffer, "Fecha:%02d/%02d/20%02d", d, mo, y);
        LCD_String_xy(0, 0, buffer);

        // Mostrar hora con prefijo "Hora:"
        sprintf(buffer, "Hora: %02d:%02d:%02d", h, m, s);
        LCD_String_xy(1, 0, buffer);

        __delay_ms(1000);
    }
}
