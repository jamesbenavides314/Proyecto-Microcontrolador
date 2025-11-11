#ifndef MATRIX8X8_H
#define MATRIX8X8_H

#include <xc.h>
#include "i2c.h"
#define _XTAL_FREQ 8000000

#define MATRIX_ADDR 0x70 << 1  // Dirección I2C del HT16K33 (ajústala si tu módulo usa otra)

void Matrix_Init(void);
void Matrix_Clear(void);
void Matrix_DisplayFace(const unsigned char *face);

// Caras predefinidas
extern const unsigned char cara_feliz[8];
extern const unsigned char cara_triste[8];

#endif
