#include "pid.h"
#include <math.h>

/**
 * @brief 将 v 限制在 [lo, hi] 范围内
 */
static float clamp(float v, float lo, float hi)
{
    if (v > hi) return hi;
    if (v < lo) return lo;
    return v;
}

/**
 * @brief 初始化 PID 控制器, 所有内部状态清零
 */
void PID_Init(PID_t *pid, PID_Mode_t mode,
              float kp, float ki, float kd,
              float out_min, float out_max,
              float integral_max)
{
    pid->mode = mode;
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->SetValue = 0.0f;
    pid->PreValue = 0.0f;

    pid->err = 0.0f;
    pid->last_err = 0.0f;
    pid->last_last_err = 0.0f;

    pid->integral = 0.0f;

    pid->out_max = out_max;
    pid->out_min = out_min;
    pid->integral_max = integral_max;
}

/**
 * @brief PID 计算核心函数
 *
 * 位置式 (PID_POSITION):
 *   out = Kp·e + Ki·∫e + Kd·(e - e1)
 *
 * 增量式 (PID_INCREMENT):
 *   Δout = Kp·(e-e1) + Ki·e + Kd·(e - 2e1 + e2)
 *   out  = 上一次输出 + Δout
 *
 * 注意: 本实现未显式乘/除 dt, 调用周期必须固定
 */
float PID_Calc(PID_t *pid, float current)
{
    // 保存历史误差, 为下一次计算准备
    pid->last_last_err = pid->last_err;
    pid->last_err = pid->err;
    pid->err = pid->SetValue - current;   // e = 目标 - 反馈

    float out = 0.0f;

    if (pid->mode == PID_POSITION) {
        // 积分累积 + 积分限幅 (抗饱和)
        pid->integral += pid->err;
        pid->integral = clamp(pid->integral, -pid->integral_max, pid->integral_max);

        // 位置式 PID: P + I + D
        out = pid->Kp * pid->err
            + pid->Ki * pid->integral
            + pid->Kd * (pid->err - pid->last_err);

    } else {
        // 增量式 PID: 计算输出增量 Δout
        float delta = pid->Kp * (pid->err - pid->last_err)
                    + pid->Ki * pid->err
                    + pid->Kd * (pid->err - 2.0f * pid->last_err + pid->last_last_err);

        // 新输出 = 上一帧输出 + 增量
        out = pid->PreValue + delta;
    }

    // 输出限幅
    out = clamp(out, pid->out_min, pid->out_max);

    pid->PreValue = out;

    return out;
}

/**
 * @brief 清零所有内部状态
 */
void PID_Reset(PID_t *pid)
{
    pid->err = 0.0f;
    pid->last_err = 0.0f;
    pid->last_last_err = 0.0f;
    pid->integral = 0.0f;
    pid->PreValue = 0.0f;
}