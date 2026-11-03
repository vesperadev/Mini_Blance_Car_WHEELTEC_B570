#ifndef __PID_H
#define __PID_H

#include "main.h"

/**
 * @brief PID 控制模式
 *   - PID_POSITION : 位置式 PID, 适用于直立环(角度)
 *   - PID_INCREMENT: 增量式 PID, 适用于速度环(编码器)
 *
 * 两种模式均未显式乘/除 dt (调用周期), 因此:
 *   - 必须在固定周期 (如定时器中断) 下调用
 *   - 实际参数 = 理论参数 × 或 ÷ dt (dt 被折算进 Kp/Ki/Kd 中)
 */
typedef enum {
    PID_POSITION,
    PID_INCREMENT
} PID_Mode_t;

/**
 * @brief PID 控制器结构体
 *
 * 使用示例 (位置式):
 *   PID_t pid_stand;
 *   PID_Init(&pid_stand, PID_POSITION, 30.0f, 0.0f, 1.2f,
 *            -7199.0f, 7199.0f, 1000.0f);
 *   pid_stand.SetValue = 0.0f;                // 目标角度
 *   float pwm = PID_Calc(&pid_stand, angle);  // 每 1~5ms 调一次
 *
 * 使用示例 (增量式):
 *   PID_t pid_speed;
 *   PID_Init(&pid_speed, PID_INCREMENT, 2.0f, 0.5f, 0.0f,
 *            -7199.0f, 7199.0f, 500.0f);
 *   pid_speed.SetValue = 0.0f;                // 目标速度
 *   float pwm = PID_Calc(&pid_speed, encoder);// 每 10~20ms 调一次
 */
typedef struct {
    float Kp;               // 比例系数
    float Ki;               // 积分系数
    float Kd;               // 微分系数

    float SetValue;         // 目标值 (每帧可改)
    float PreValue;         // 上一帧输出值 (增量式用, 也用作输出限幅的保存)

    float err;              // 当前误差
    float last_err;         // 上一次误差
    float last_last_err;    // 上上次误差 (增量式微分项需要)

    float integral;         // 累积积分项 (位置式用, 带限幅)

    float out_max;          // 输出上限 (比如 PWM 最大值 7199)
    float out_min;          // 输出下限
    float integral_max;     // 积分项限幅 (防止积分饱和)

    PID_Mode_t mode;        // 控制模式: 位置式 / 增量式
} PID_t;

/**
 * @brief 初始化 PID 控制器
 * @param pid           控制器指针
 * @param mode          控制模式
 * @param kp, ki, kd    PID 三个系数
 * @param out_min       输出下限
 * @param out_max       输出上限
 * @param integral_max  积分项限幅 (仅位置式有效, 增量式传 0 即可)
 */
void PID_Init(PID_t *pid, PID_Mode_t mode,
              float kp, float ki, float kd,
              float out_min, float out_max,
              float integral_max);

/**
 * @brief PID 计算 (必须固定周期调用)
 * @param pid      控制器指针
 * @param current  当前测量值 (传感器反馈)
 * @return         PID 输出值 (已限幅)
 */
float PID_Calc(PID_t *pid, float current);

/**
 * @brief 重置 PID 内部状态 (切换模式或紧急停机时调用)
 * @param pid 控制器指针
 */
void PID_Reset(PID_t *pid);

#endif /* __PID_H */