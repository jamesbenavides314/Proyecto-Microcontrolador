<<<<<<< HEAD
#ifndef I2C_H
#define I2C_H

void I2C_Master_Init(const unsigned long c);
void I2C_Master_Wait(void);
void I2C_Master_Start(void);
void I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
unsigned char I2C_Master_Write(unsigned d);
unsigned short I2C_Master_Read(unsigned short a);

#endif
=======
#ifndef I2C_H
#define I2C_H

void I2C_Master_Init(const unsigned long c);
void I2C_Master_Wait(void);
void I2C_Master_Start(void);
void I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
unsigned char I2C_Master_Write(unsigned d);
unsigned short I2C_Master_Read(unsigned short a);

#endif
>>>>>>> 8d551b62a202ddcce05a518ee4d0d2b4dcccc57f
