#include "config.h"  // Descomenta si usas archivos separados
#include "hw498.h"
#include "adc.h"
#include <math.h>

void HW498_Init(void) {
    TRISAbits.TRISA0 = 1;
}

float HW498_LeerTemperatura(void) {
    unsigned int valorADC;
    float voltaje;
    float resistencia;
    float temperatura;
    
    // Parámetros del HW-498 (NTC 10k)
    const float R0 = 10000.0;        // Resistencia a 25°C (10k?)
    const float T0 = 298.15;         // 25°C en Kelvin
    const float BETA = 3950.0;       // Coeficiente Beta del NTC
    const float R_SERIE = 10000.0;   // Resistencia en serie del módulo (10k?)
    
    // Leer valor ADC del sensor
    valorADC = ADC_Read(HW498_CANAL);
    
    // Evitar división por cero
    if(valorADC == 0) valorADC = 1;
    if(valorADC >= 1023) valorADC = 1022;
    
    // Convertir ADC a voltaje (0-5V)
    voltaje = (valorADC * 5.0) / 1023.0;
    
    // Calcular resistencia del NTC usando divisor de voltaje
    // Vout = Vcc * (R_NTC / (R_SERIE + R_NTC))
    // Despejando: R_NTC = R_SERIE * Vout / (Vcc - Vout)
    resistencia = R_SERIE * voltaje / (5.0 - voltaje);
    
    // Ecuación simplificada de Steinhart-Hart (aproximación Beta)
    // 1/T = 1/T0 + (1/BETA) * ln(R/R0)
    // T = 1 / (1/T0 + (1/BETA) * ln(R/R0))
    float lnR = log(resistencia / R0);
    float temperaturaK = 1.0 / ((1.0 / T0) + (lnR / BETA));
    
    // Convertir de Kelvin a Celsius
    temperatura = temperaturaK - 273.15;
    
    // Limitar rango razonable
    if(temperatura > 125.0) temperatura = 125.0;
    if(temperatura < -40.0) temperatura = -40.0;
    
    return temperatura;
}