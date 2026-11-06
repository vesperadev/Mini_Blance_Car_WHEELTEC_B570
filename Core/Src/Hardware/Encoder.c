#include "main.h"
#include "tim.h"

/**************************************************************************
函数功能：启动编码器接口 (TIM3->左轮A, TIM4->右轮B)
入口参数：无
返回  值：无
说明    ：必须在 MX_TIM3_Init / MX_TIM4_Init 之后调用; 只开启计数,
          实际读数在 Encoder_Update 里完成。
          左轮已改到 TIM3 (PA6/PA7); 右轮仍为 TIM4 (PB6/PB7)。
**************************************************************************/
void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

int16_t Encoder_GetCountA = 0;
int16_t Encoder_GetCountB = 0;

/**************************************************************************
函数功能：读取左右编码器单周期脉冲数, 并清零计数器
入口参数：无
返回  值：无
说明    ：建议在固定周期 (如 5~10ms 定时器中断) 中调用。
          电机 B 绑定编码器 4 (TIM4, PB6=CH1/PB7=CH2, X4 计数), 方向已取反。
          电机 A 绑定编码器 3 (TIM3, PA6=CH1/PA7=CH2, X4 计数)。
          若某侧方向仍与转向相反, 对该侧读数取负号即可。
**************************************************************************/
void Encoder_Update(void)
{
    // ---- 电机 B / 右轮 -> 编码器 4 (TIM4) ----
    // 实测方向反了, 此处取负校正
    Encoder_GetCountB = -(int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    __HAL_TIM_SET_COUNTER(&htim4, 0);   // 清零, 下次读取为本周期增量

    // ---- 电机 A / 左轮 -> 编码器 3 (TIM3) ----
    // 实测方向也反了, 取负校正
    Encoder_GetCountA = -(int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}
