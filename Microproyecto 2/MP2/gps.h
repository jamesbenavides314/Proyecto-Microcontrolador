#ifndef GPS_H
#define GPS_H

#include <xc.h>

#define _XTAL_FREQ 8000000

// --- Prototipos ---
void GPS_Init(void);
char GPS_ReadLine(char *buffer);
void GPS_GetCoordinates(char *lat, char *lon);

#endif
