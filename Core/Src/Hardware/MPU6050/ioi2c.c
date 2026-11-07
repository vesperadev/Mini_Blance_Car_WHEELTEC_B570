#include "ioi2c.h"
#include "i2c.h"

#define MPU_I2C_TIMEOUT  100U

uint8_t Flag_Show = 0;

/**************************************************************************
说明：DMP 库传入 7 位地址 (0x68), HAL_I2C_Mem_* 需要 8 位地址 (0x68<<1=0xD0);
      MPU6050.c 传入的 dev 本身已是 0xD0, 直接使用。
**************************************************************************/
uint8_t i2cWrite(uint8_t addr, uint8_t reg, uint8_t len, uint8_t const *data)
{
    return (uint8_t)(HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(addr << 1), reg,
                     I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len, MPU_I2C_TIMEOUT) != HAL_OK);
}

uint8_t i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{
    return (uint8_t)(HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr << 1), reg,
                     I2C_MEMADD_SIZE_8BIT, data, len, MPU_I2C_TIMEOUT) != HAL_OK);
}

static uint8_t MPU_ReadReg(uint8_t dev, uint8_t reg)
{
    uint8_t val = 0;
    HAL_I2C_Mem_Read(&hi2c1, dev, reg, I2C_MEMADD_SIZE_8BIT,
                     &val, 1, MPU_I2C_TIMEOUT);
    return val;
}

static uint8_t MPU_WriteReg(uint8_t dev, uint8_t reg, uint8_t val)
{
    return (uint8_t)(HAL_I2C_Mem_Write(&hi2c1, dev, reg, I2C_MEMADD_SIZE_8BIT,
                     &val, 1, MPU_I2C_TIMEOUT) != HAL_OK);
}

int8_t IICwriteByte(uint8_t dev, uint8_t regAddr, uint8_t data)
{
    return MPU_WriteReg(dev, regAddr, data) ? -1 : 0;
}

int8_t IICwriteBit(uint8_t dev, uint8_t regAddr, uint8_t bitNum, uint8_t data)
{
    uint8_t b = MPU_ReadReg(dev, regAddr);
    b = (data != 0) ? (uint8_t)(b | (1u << bitNum))
                    : (uint8_t)(b & ~(1u << bitNum));
    return MPU_WriteReg(dev, regAddr, b) ? -1 : 0;
}

int8_t IICwriteBits(uint8_t dev, uint8_t regAddr,
                    uint8_t bitStart, uint8_t length, uint8_t data)
{
    uint8_t b = MPU_ReadReg(dev, regAddr);
    uint8_t shift = bitStart - length + 1;
    uint8_t mask = (uint8_t)(((1u << length) - 1u) << shift);
    b = (uint8_t)(b & ~mask);
    b = (uint8_t)(b | ((uint8_t)(data << shift) & mask));
    return MPU_WriteReg(dev, regAddr, b) ? -1 : 0;
}

int8_t IICreadBytes(uint8_t dev, uint8_t regAddr, uint8_t length, uint8_t *data)
{
    return (HAL_I2C_Mem_Read(&hi2c1, dev, regAddr, I2C_MEMADD_SIZE_8BIT,
             data, length, MPU_I2C_TIMEOUT) == HAL_OK) ? 0 : -1;
}

uint8_t I2C_ReadOneByte(uint8_t dev, uint8_t regAddr)
{
    return MPU_ReadReg(dev, regAddr);
}
