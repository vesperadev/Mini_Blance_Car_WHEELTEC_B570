#include "main.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"   // HAL_TIM_SetCompare / HAL_TIM_PWM_Start
#include <stdlib.h>              // abs
void Motor_Init(void)
{
    // 初始化电机引脚 -- 由MX已经完成 
    // PWM 初始化 -- 开启PWM输出
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

void Motor_SetPWM(int16_t PWMB, int16_t PWMA)
{
    PWMA = -PWMA;
    // ---- 电机 A 方向 (AIN1=PB12, AIN2=PB13) ----
    if (PWMA >= 0) {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET);
    }
    // ---- 电机 B 方向 (BIN1=PB14, BIN2=PB15) ----
    if (PWMB >= 0) {
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET);
    }

    // ---- 设置 PWM 占空比 (取绝对值, 范围 0 ~ 7199) ----
    // 电机 A PWM -> TIM1_CH4 (PA11)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, (uint32_t)abs(PWMA));
    // 电机 B PWM -> TIM1_CH1 (PA8)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)abs(PWMB));
}