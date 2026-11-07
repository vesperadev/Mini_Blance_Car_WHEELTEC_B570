/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Hardware/LED.h"
#include "Hardware/Blue.h"
#include "Hardware/OLED.h"
#include "Hardware/Motor.h"
#include "Hardware/Encoder.h"
#include "Hardware/MPU6050/mpu6050.h"
#include "Hardware/MPU6050/inv_mpu.h"
#include "Hardware/MPU6050/inv_mpu_dmp_motion_driver.h"
#include "Algorithm/pid.h"
#include <math.h>
#include "Hardware/Encoder.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
PID_t pid_stand;   // 直立环 (位置式, PD)
PID_t pid_speed;   // 速度环 (增量式, PI)
PID_t pid_turn;    // 转向环 (位置式, P 就行)`
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ================= MPU6050 / DMP 诊断 ================= */
volatile uint8_t mpu_dmp_ready = 0;   // EXTI 中断置位, 主循环消费
uint8_t  mpu_who      = 0;    // WHO_AM_I 读回值 (应为 0x68)
int      mpu_fifo_ret = -1;  // dmp_read_fifo 返回值 (0 = 成功)
short    mpu_sensors  = 0;    // DMP sensors 掩码
uint8_t  mpu_more     = 0;    // FIFO 仍有剩余标志

#define MPU_Q30  1073741824.0f

/* 读一次 DMP FIFO 并更新姿态角, 同时记录诊断信息 */
static void MPU_Diag_Update(void)
{
    long quat[4] = {0, 0, 0, 0};
    unsigned long ts = 0;
    unsigned char more = 0;
    short sensors = 0;

    /* 1) 直接读 WHO_AM_I, 确认 I2C 通路 */
    mpu_who = MPU6050_getDeviceID();

    /* 2) 读 DMP FIFO (返回值反映 DMP 是否真的在工作) */
    mpu_fifo_ret = dmp_read_fifo(gyro, accel, quat, &ts, &sensors, &more);
    mpu_sensors  = sensors;
    mpu_more     = more;

    /* 3) 只有拿到有效四元数才更新姿态角, 避免用栈垃圾值算出假角度 */
    if ((mpu_fifo_ret == 0) && (sensors & INV_WXYZ_QUAT)) {
        float q0 = quat[0] / MPU_Q30;
        float q1 = quat[1] / MPU_Q30;
        float q2 = quat[2] / MPU_Q30;
        float q3 = quat[3] / MPU_Q30;
        Roll  = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3f;
        Pitch = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3f;
        Yaw   = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3f;
    }
}

/* OLED 工具: 打印 1 个十六进制字节 */
static void OLED_ShowHex8(uint8_t x, uint8_t y, uint8_t val)
{
    OLED_ShowChar(x,     y, "0123456789ABCDEF"[val >> 4], 12, 1);
    OLED_ShowChar(x + 6, y, "0123456789ABCDEF"[val & 0x0F], 12, 1);
}

/* OLED 工具: 带符号十进制显示 */
static void OLED_ShowSigned(uint8_t x, uint8_t y, int32_t val, uint8_t len)
{
    if (val < 0) {
        OLED_ShowChar(x, y, '-', 12, 1);
        OLED_ShowNumber(x + 8, y, (u32)(-val), len, 12);
    } else {
        OLED_ShowChar(x, y, ' ', 12, 1);
        OLED_ShowNumber(x + 8, y, (u32)val, len, 12);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  Blue_Init();
  OLED_Init();
  Motor_Init();
  Encoder_Init();
  MPU6050_initialize();
  DMP_Init();
  Motor_SetPWM(0, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t tick_oled = HAL_GetTick();
  while (1)
  {
    /* MPU6050: 中断只置标志, DMP 在这里读 (不能在中断里跑 HAL_I2C) */
    if (mpu_dmp_ready) {
        mpu_dmp_ready = 0;
        MPU_Diag_Update();
    }

    /* OLED 每 1s 刷新一次 */
    if (HAL_GetTick() - tick_oled >= 1000) {
        tick_oled = HAL_GetTick();
        Encoder_Update();
        OLED_Clear();

        /* 行1: 自检 -> WHO_AM_I | dmp_read_fifo 返回值 | sensors 掩码 */
        OLED_ShowString(0, 0, "W");
        OLED_ShowHex8(12, 0, mpu_who);
        OLED_ShowString(20, 0, "F");
        OLED_ShowNumber(26, 0, (u32)mpu_fifo_ret, 3, 12);
        OLED_ShowString(44, 0, "S");
        OLED_ShowHex8(50, 0, (uint8_t)mpu_sensors);

        /* 行2: 姿态角 Pitch / Roll */
        OLED_ShowString(0, 12, "P");
        OLED_ShowSigned(8, 12, (int32_t)Pitch, 3);
        OLED_ShowString(52, 12, "R");
        OLED_ShowSigned(60, 12, (int32_t)Roll, 3);

        /* 行3: 原始数据 gyro[0] / accel[0] */
        OLED_ShowString(0, 24, "G");
        OLED_ShowSigned(8, 24, (int32_t)gyro[0], 5);
        OLED_ShowString(52, 24, "A");
        OLED_ShowSigned(60, 24, (int32_t)accel[0], 5);

        /* 行4/5: 编码器计数 */
        OLED_ShowString(0, 36, "EA");
        OLED_ShowSigned(18, 36, (int32_t)Encoder_GetCountA, 5);
        OLED_ShowString(0, 48, "EB");
        OLED_ShowSigned(18, 48, (int32_t)Encoder_GetCountB, 5);

        OLED_Refresh_Gram();   // 把 GRAM 推送到屏幕，否则只写缓冲、屏一直全黑
    }
    HAL_Delay(1);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
