#include "config.h"
#include "adc.h"

void ADC_Init(void) {
    // Configuración CORREGIDA para solo AN0 analógico
    ADCON1bits.PCFG = 0b1110;  // AN0 analógico, resto digital
    ADCON1bits.VCFG0 = 0;      // Vref+ = VDD
    ADCON1bits.VCFG1 = 0;      // Vref- = VSS
    
    ADCON2bits.ACQT = 0b010;   // Acquisition time = 4 TAD
    ADCON2bits.ADCS = 0b001;   // Fosc/8
    ADCON2bits.ADFM = 1;       // Resultado justificado a la derecha
    
    ADCON0bits.ADON = 1;       // Encender módulo ADC
    __delay_us(50);            // Tiempo de estabilización
}

unsigned int ADC_Read(unsigned char canal) {
    unsigned int resultado;
    
    ADCON0bits.CHS = canal;    // Seleccionar canal
    __delay_us(20);            // Tiempo de adquisición
    
    ADCON0bits.GO_DONE = 1;    // Iniciar conversión
    while(ADCON0bits.GO_DONE); // Esperar a que termine
    
    resultado = ((unsigned int)ADRESH << 8) | ADRESL;
    return resultado;
}