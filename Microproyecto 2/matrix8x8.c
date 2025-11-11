#include "matrix8x8.h"

// --- Datos de las caras ---
const unsigned char cara_feliz[8] = {
    0b00000000,
    0b00100100,
    0b00100100,
    0b00000000,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00000000
};

const unsigned char cara_triste[8] = {
    0b00000000,
    0b00100100,
    0b00100100,
    0b00000000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b00000000
};

void Matrix_Init(void) {
    I2C_Master_Start();
    I2C_Master_Write(MATRIX_ADDR);
    I2C_Master_Write(0x21); // Oscillator ON
    I2C_Master_Stop();

    I2C_Master_Start();
    I2C_Master_Write(MATRIX_ADDR);
    I2C_Master_Write(0x81); // Display ON, blink OFF
    I2C_Master_Stop();

    I2C_Master_Start();
    I2C_Master_Write(MATRIX_ADDR);
    I2C_Master_Write(0xE7); // Full brightness
    I2C_Master_Stop();

    Matrix_Clear();
}

void Matrix_Clear(void) {
    I2C_Master_Start();
    I2C_Master_Write(MATRIX_ADDR);
    I2C_Master_Write(0x00); // Empezar en dirección 0
    for (unsigned char i = 0; i < 16; i++) {
        I2C_Master_Write(0x00);
    }
    I2C_Master_Stop();
}

void Matrix_DisplayFace(const unsigned char *face) {
    I2C_Master_Start();
    I2C_Master_Write(MATRIX_ADDR);
    I2C_Master_Write(0x00); // Dirección inicial
    for (unsigned char i = 0; i < 8; i++) {
        I2C_Master_Write(face[i]); // Byte de fila
        I2C_Master_Write(0x00);    // Espaciado requerido por HT16K33
    }
    I2C_Master_Stop();
}
