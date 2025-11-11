#include "gps.h"
#include <string.h>
#include <stdio.h>

#define GPS_TIMEOUT_MS 2000  // Tiempo máximo de espera (2 segundos)

void GPS_Init(void) {
    TRISC6 = 0; // TX
    TRISC7 = 1; // RX
    SPBRG = ((_XTAL_FREQ / 64) / 9600) - 1;
    TXSTAbits.BRGH = 0;
    TXSTAbits.SYNC = 0;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1;
}

// --- Leer un carácter del GPS con tiempo límite ---
char GPS_ReadCharTimeout(unsigned int timeout_ms) {
    unsigned int i;
    for (i = 0; i < timeout_ms; i++) {
        if (RCIF) return RCREG;  // Si hay dato, retornar
        __delay_ms(1);
    }
    return 0;  // *** Retorna 0 si no llega nada ***
}

// --- Leer una línea NMEA (termina en '\n') ---
char GPS_ReadLine(char *buffer) {
    char c;
    unsigned int i = 0;

    // *** Primer carácter con timeout (para detectar GPS desconectado) ***
    c = GPS_ReadCharTimeout(GPS_TIMEOUT_MS);
    if (c == 0) {
        buffer[0] = '\0'; // No llegó nada
        return 0;         // *** Indica fallo ***
    }

    while (1) {
        if (c == '\n') {
            buffer[i] = '\0';
            break;
        } else if (i < 99) {
            buffer[i++] = c;
        }
        c = GPS_ReadCharTimeout(50);  // Pequeño timeout entre caracteres
        if (c == 0) break;  // *** Si se corta la comunicación ***
    }
    return 1; // *** Lectura exitosa ***
}

// --- Extraer latitud y longitud desde $GPRMC ---
void GPS_GetCoordinates(char *lat, char *lon) {
    char line[100];
    char *token;

    if (!GPS_ReadLine(line)) {
        // *** GPS no responde ***
        strcpy(lat, "NO GPS");
        strcpy(lon, "NO GPS");
        return;
    }

    if (strstr(line, "$GPRMC")) {
        token = strtok(line, ",");
        for (int i = 0; i < 3 && token != NULL; i++)
            token = strtok(NULL, ",");

        if (token) strcpy(lat, token); // Latitud
        token = strtok(NULL, ",");     // N/S
        if (token) strcat(lat, token);

        token = strtok(NULL, ",");     // Longitud
        if (token) strcpy(lon, token);
        token = strtok(NULL, ",");     // E/W
        if (token) strcat(lon, token);
    } else {
        strcpy(lat, "----");
        strcpy(lon, "----");
    }
}
