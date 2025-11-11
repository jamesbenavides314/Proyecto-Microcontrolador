#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "adc.h"
#include "lcd.h"
#include "hw498.h"
#include "i2c.h"
#include "ds1307.h"
#include "gps.h"
#include "matrix8x8.h"

#define _XTAL_FREQ 8000000

volatile unsigned char sistema_pausado = 0;  // 0 = activo, 1 = pausado

void Config_Oscilador(void) {
    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 0b10;
    while(!OSCCONbits.IOFS);
}

// --- Interrupción externa para el botón ---
void __interrupt() ISR(void) {
    if (INTCON3bits.INT2IF) {   // Si ocurrió interrupción externa INT2 (RB2)
        __delay_ms(50); // Anti-rebote
        if (!PORTBbits.RB2) {   // Confirmar pulsación (botón activo en bajo)
            sistema_pausado = !sistema_pausado;  // Cambiar estado del sistema
        }
        INTCON3bits.INT2IF = 0; // Limpiar bandera de INT2
    }
}


void EsperarConChequeo(unsigned int tiempo_ms) {
    unsigned int i;
    for (i = 0; i < tiempo_ms / 10; i++) {  // Espera en pasos de 10 ms
        __delay_ms(10);
        if (sistema_pausado) return;  // Si se presiona el botón, se pausa
    }
}

void main(void) {
    float temperatura;
    unsigned char h, m, s, d, mo, y;
    char buffer[17];
    char lat[16], lon[16];

    // --- Configuración de pines CORREGIDA ---
    TRISAbits.TRISA0 = 1;  // RA0 entrada analógica (HW-498)
    TRISBbits.TRISB2 = 1;  // RB2 como entrada (botón) ? CAMBIO DE RB0 A RB2
    
    // Configurar ADCON1 correctamente
    ADCON1 = 0x0E;  // Solo AN0 (RA0) como analógico, resto digital

    // --- Configuración interrupción externa INT0 en RB2 ---
    // --- Configuración interrupción externa INT2 en RB2 ---
    TRISBbits.TRISB2 = 1;      // RB2 como entrada (botón)
    INTCON2bits.RBPU = 0;      // Activar resistencias pull-up en PORTB
    INTCON2bits.INTEDG2 = 0;   // Interrupción en flanco descendente (presionar)
    INTCON3bits.INT2IF = 0;    // Limpiar bandera
    INTCON3bits.INT2IE = 1;    // Habilitar INT2
    INTCONbits.PEIE = 1;       // Habilitar interrupciones periféricas
    INTCONbits.GIE = 1;        // Habilitar interrupciones globales


    // --- Inicializaciones ---
    Config_Oscilador();
    ADC_Init();
    LCD_Init();
    HW498_Init();
    I2C_Master_Init(100000);
    RTC_Init();
    //RTC_SetDateTime(15, 35, 0, 11, 11, 25);   Configuración de hora y fecha, solo se carga una vez 
    GPS_Init();
    Matrix_Init();

    LCD_SetCursor(1, 0);
    LCD_String("Proyecto Sensor");
    LCD_SetCursor(2, 0);
    LCD_String("Iniciando...");
    __delay_ms(2000);
    LCD_Clear();

   while(1) {
        if (sistema_pausado == 0) {
            // --- Sistema activo ---
            Matrix_DisplayFace(cara_feliz);

            // Leer temperatura
            temperatura = HW498_LeerTemperatura();
            LCD_SetCursor(1, 0);
            LCD_String("Temp:");
            LCD_SetCursor(1, 6);
            LCD_Float(temperatura, 1);
            LCD_String("C");
            EsperarConChequeo(5000);
            if (sistema_pausado) continue;
            LCD_Clear();

            // Leer fecha y hora
            RTC_GetDateTime(&h, &m, &s, &d, &mo, &y);
            sprintf(buffer, "%02d/%02d/20%02d", d, mo, y);
            LCD_String_xy(0, 0, buffer);
            sprintf(buffer, "%02d:%02d:%02d", h, m, s);
            LCD_String_xy(1, 0, buffer);
            EsperarConChequeo(5000);
            if (sistema_pausado) continue;
            LCD_Clear();

            // Leer coordenadas GPS
            // Leer coordenadas GPS
            GPS_GetCoordinates(lat, lon);

            if (strcmp(lat, "NO GPS") == 0) {
                LCD_String_xy(0, 0, "GPS no detectado");
                LCD_String_xy(1, 0, "Sin senal");
            }
            else if (strcmp(lat, "----") == 0 || strcmp(lon, "----") == 0 || strlen(lat) < 4) {
                // Sin señal satelital
                LCD_String_xy(0, 0, "Sin senal GPS");
                LCD_String_xy(1, 0, "Buscando...");
            } 
            else {
                // Señal válida
                LCD_String_xy(0, 0, "Lat:");
                LCD_String_xy(0, 4, lat);
                LCD_String_xy(1, 0, "Lon:");
                LCD_String_xy(1, 4, lon);
            }
            LCD_Clear();


        } else {
            // --- Sistema pausado ---
            LCD_Clear();
            LCD_String_xy(0, 0, "Sistema pausado");
            Matrix_DisplayFace(cara_triste);

            // Espera hasta que se reanude
            while (sistema_pausado) {
                __delay_ms(300);
            }

            LCD_Clear();
        }
    }
}