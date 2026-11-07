#include "delay.h"

/* 移植层: 用 HAL 的 SysTick 提供 DMP 库需要的毫秒延时/时间戳 */
void delay_ms(unsigned long ms)
{
    HAL_Delay(ms);
}

void get_ms(unsigned long *time)
{
    if (time) *time = HAL_GetTick();
}
