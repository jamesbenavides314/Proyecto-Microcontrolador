#pragma config FOSC = INTOSCIO_EC
#pragma config FCMEN = OFF
#pragma config PWRT = OFF
#pragma config BOR = ON
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config CP0 = OFF
#pragma config WRT0 = OFF
#pragma config MCLRE = OFF

#include <xc.h>
#include <pic18f4550.h>

#define _XTAL_FREQ 8000000

int main() {
    
    OSCCON=0x72;
    TRISB = 0;
    while(1){
            LATBbits.LATB0= 1;
            __delay_ms(500);

            LATBbits.LATB0 = 0;
            __delay_ms(4375);
    }
}

