#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <pic18f4550.h>

// Configuración de bits
#pragma config FOSC = INTOSCIO_EC
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF
#pragma config MCLRE = OFF

#define _XTAL_FREQ 8000000

#endif