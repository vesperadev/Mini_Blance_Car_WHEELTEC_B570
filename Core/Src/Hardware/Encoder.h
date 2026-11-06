#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"

// 编码器计数 (每次 Encoder_Update 刷新为单周期增量, 单位: 脉冲*4)
//   A: 左轮, 绑定 TIM3 (PA6/PA7)
//   B: 右轮, 绑定 TIM4 (PB6/PB7), 方向已在 Encoder_Update 中取反校正
extern int16_t Encoder_GetCountA;
extern int16_t Encoder_GetCountB;

void Encoder_Init(void);
void Encoder_Update(void);

#endif /* __ENCODER_H */
