#ifndef __IOI2C_H
#define __IOI2C_H

/* 移植层: 用硬件 I2C1 (hi2c1, PB8/PB9, 400kHz) 实现官方软件 I2C 的接口 */
#include "sys.h"

/* 给 DMP 库 (inv_mpu.c) 使用, addr 为 7 位器件地址 0x68 */
uint8_t i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t const *data);
uint8_t i2cRead (uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);

/* 给 MPU6050.c 使用, dev 为 8 位地址 0xD0 (0x68<<1) */
int8_t  IICwriteBits  (uint8_t dev, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
int8_t  IICwriteBit   (uint8_t dev, uint8_t regAddr, uint8_t bitNum, uint8_t data);
int8_t  IICwriteByte  (uint8_t dev, uint8_t regAddr, uint8_t data);
int8_t  IICreadBytes  (uint8_t dev, uint8_t regAddr, uint8_t length, uint8_t *data);
uint8_t I2C_ReadOneByte(uint8_t dev, uint8_t regAddr);

/* MPU6050.c 的 DMP_Init 用到 (原例程在 show.c 中定义) */
extern uint8_t Flag_Show;

#endif /* __IOI2C_H */
