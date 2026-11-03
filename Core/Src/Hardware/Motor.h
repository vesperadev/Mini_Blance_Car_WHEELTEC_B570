#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

// 电机方向引脚 (CubeMX 标签, 均在 GPIOB):
//   AIN1 = PB12, AIN2 = PB13, BIN1 = PB14, BIN2 = PB15
void Motor_Init(void);
void Motor_SetPWM(int16_t PWMA, int16_t PWMB);
#endif /* __MOTOR_H */
