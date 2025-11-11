<<<<<<< HEAD
#include <xc.h>
#define _XTAL_FREQ 8000000

void I2C_Master_Init(const unsigned long c) {
    TRISB0 = 1; // SDA
    TRISB1 = 1; // SCL
    SSPCON1 = 0b00101000; // Enable MSSP in I2C Master mode
    SSPCON2 = 0;
    SSPADD = (_XTAL_FREQ / (4 * c)) - 1; // Set clock speed
    SSPSTAT = 0;
}

void I2C_Master_Wait(void) {
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void I2C_Master_Start(void) {
    I2C_Master_Wait();
    SEN = 1;
}

void I2C_Master_RepeatedStart(void) {
    I2C_Master_Wait();
    RSEN = 1;
}

void I2C_Master_Stop(void) {
    I2C_Master_Wait();
    PEN = 1;
}

unsigned char I2C_Master_Write(unsigned d) {
    I2C_Master_Wait();
    SSPBUF = d;
    I2C_Master_Wait();
    return !ACKSTAT; // ACK = 0 ? correcto; devuelve 1 si hubo ACK
}

unsigned short I2C_Master_Read(unsigned short a) {
    unsigned short temp;
    I2C_Master_Wait();
    RCEN = 1;
    I2C_Master_Wait();
    temp = SSPBUF;
    I2C_Master_Wait();
    ACKDT = (a)?0:1;
    ACKEN = 1;
    return temp;
}
=======
#include <xc.h>
#include <pic18f4550.h>
#include <stdint.h>
#include "Config.h"

#define _XTAL_FREQ 8000000

void I2C_Master_Init(const unsigned long c) {
    
    TRISB0 = 1; // SCL
    TRISB1 = 1; // SDA
    SSPCON1 = 0b00101000; // Enable MSSP in I2C Master mode
    SSPCON2 = 0;
    SSPADD = (_XTAL_FREQ / (4 * c)) - 1; // Set clock speed
    SSPSTAT = 0;
}

void I2C_Master_Wait(void) {
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void I2C_Master_Start(void) {
    I2C_Master_Wait();
    SSPCON2bits.SEN = 1;
}

void I2C_Master_RepeatedStart(void) {
    I2C_Master_Wait();
    SSPCON2bits.RSEN = 1;
}

void I2C_Master_Stop(void) {
    I2C_Master_Wait();
    SSPCON2bits.PEN = 1;
}

unsigned char I2C_Master_Write(unsigned d) {
    I2C_Master_Wait();
    SSPBUF = d;
    I2C_Master_Wait();
    return !SSPCON2bits.ACKSTAT; // ACK = 0 ? correcto; devuelve 1 si hubo ACK
}

unsigned short I2C_Master_Read(unsigned short a) {
    unsigned short temp;
    I2C_Master_Wait();
    SSPCON2bits.RCEN = 1;
    I2C_Master_Wait();
    temp = SSPBUF;
    I2C_Master_Wait();
    SSPCON2bits.ACKDT = (a)?0:1;
    SSPCON2bits.ACKEN = 1;
    return temp;
}
>>>>>>> 8d551b62a202ddcce05a518ee4d0d2b4dcccc57f
