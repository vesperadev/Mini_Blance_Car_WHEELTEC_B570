# Mini_Blance_Car_WHEELTEC_B570

WHEELTEC B570 平衡小车固件，基于 **STM32F103C8T6**（Cortex-M3，64 KB Flash / 20 KB RAM）。
工程由 STM32CubeMX 生成，使用 **CMake + Ninja + arm-none-eabi-gcc** 构建，通过 **OpenOCD + 无线 CMSIS-DAP 调试器** 烧录与在线调试。

---

## 1. 硬件平台

| 项 | 说明 |
|----|------|
| 主控 | STM32F103C8T6（OpenOCD 实测 `device id = 0x20036410`，flash 64 KiB） |
| 系统时钟 | HSE 8 MHz + PLL×9 → 72 MHz（`SystemClock_Config`） |
| 调试器 | Horco CMSIS-DAP v2 无线调试器（VID:PID `0xfaed:0x4873`，串口 COM3 / MSC 盘） |
| 屏幕 | 0.96″ OLED，4 线软件 SPI（SSD1306 序列） |
| 其它 | 编码器 ×2、直流电机 ×2、MPU6050、蓝牙模块（USART3）、LED、按键、电池 ADC |

> 烧录用 OpenOCD 0.12（xpack），**不需要 ST-Link，也不需要 pyOCD**。STM32CubeProgrammer 不支持 CMSIS-DAP，请勿使用。

---

## 2. 目录结构

```
.
├── CMakeLists.txt                # 顶层 CMake（CubeMX 生成）
├── CMakePresets.json             # Debug / Release 预设
├── Mini_Blance_Car_WHEELTEC_B570.ioc   # CubeMX 工程
├── STM32F103XX_FLASH.ld          # 链接脚本
├── startup_stm32f103xb.s        # 启动文件
├── openocd.cfg                   # OpenOCD 调试/烧录配置（无线 CMSIS-DAP）
├── Core/
│   ├── Src/
│   │   ├── main.c                # 主循环：OLED 显示 "Hello, World!"
│   │   ├── gpio.c / usart.c / dma.c
│   │   └── Hardware/
│   │       ├── OLED.c / OLED.h / oledfont.h   # 4 线 SPI OLED 驱动
│   │       ├── Blue.c / Blue.h                # 蓝牙 USART3 + DMA 空闲中断
│   │       └── LED.h                          # LED 控制宏
│   └── Inc/ main.h …
├── Drivers/                      # STM32 HAL / CMSIS
└── cmake/
    ├── stm32cubemx/CMakeLists.txt
    ├── gcc-arm-none-eabi.cmake   # 工具链（要求 arm-none-eabi-* 在 PATH）
    └── starm-clang.cmake
```

---

## 3. 构建

### 方式 A：VS Code（推荐）
安装扩展 **STM32 相关扩展** 与 **CMake Tools**，打开本工程后：
- `Ctrl+Shift+P` → `CMake: Configure`（首次会自动生成 `build/Debug`）
- `Ctrl+Shift+B` 构建（默认任务 `Build`，等价于 `cmake --build build/Debug`）

### 方式 B：命令行
需先在 PATH 中放入 STM32Cube 自带的 `arm-none-eabi-gcc` 与 `cmake`/`ninja`：

```powershell
# 配置（仅首次，或改了 .ioc 之后）
cmake --preset Debug

# 增量构建
cmake --build build/Debug
```

> 本机已验证的构建命令（见 `.vscode/tasks.json` 的 `Build` 任务）：
> `C:/Users/37963/AppData/Local/stm32cube/bundles/cmake/4.3.1+st.1/bin/cmake.exe --build build/Debug`

产物：`build/Debug/Mini_Blance_Car_WHEELTEC_B570.elf`

---

## 4. 烧录与调试（OpenOCD + 无线 CMSIS-DAP）

已配好三个文件，无需额外操作：

| 文件 | 作用 |
|------|------|
| `openocd.cfg` | 接口 `cmsis-dap` + SWD 1000 kHz + `target/stm32f1x.cfg`；`reset_config none`（无线只接 4 线 SWD，无 nRST，用 SYSRESETREQ 软复位） |
| `.vscode/launch.json` | 两条 Cortex-Debug 配置：`Debug`（F5 自动构建+下载+停在 main）/`Attach` |
| `.vscode/tasks.json` | `Build` 构建、`Flash (OpenOCD 无线)` 构建并烧录校验 |

**调试**：插好无线调试器 → F5 → 选 `Debug OpenOCD (无线 CMSIS-DAP)`。
**只烧录**：`Ctrl+Shift+P` → Run Task → `Flash (OpenOCD 无线)`。

命令行等价烧录：

```powershell
openocd -f openocd.cfg -c "program build/Debug/Mini_Blance_Car_WHEELTEC_B570.elf verify reset exit"
```

> 排错：残留的 OpenOCD 会占用 3333(gdb)/4444(telnet) 端口，导致 F5 起服务失败。
> 清理：`Get-Process openocd` → `Stop-Process -Id <pid> -Force`。

---

## 5. 外设驱动现状

| 模块 | 状态 | 说明 |
|------|------|------|
| OLED（4 线 SPI） | ✅ 已实现 | 显存缓冲：`OLED_ShowString/ShowChar/ShowNumber/DrawPoint` 只写 `OLED_GRAM[128][8]`，**必须调用 `OLED_Refresh_Gram()` 才会刷新到屏** |
| 蓝牙（USART3 + DMA 空闲中断） | ✅ 已实现 | `Blue_Init()` 启动 DMA 接收，`HAL_UARTEx_RxEventCallback` 回显 |
| LED | ✅ 已实现 | `LED.h` 宏：`LED_ON()/LED_OFF()/LED_TOGGLE()/LED_State()` |
| 编码器 / 电机 PWM / 电机方向 | ⬜ 待实现 | 见下方接线，尚未编写驱动 |
| MPU6050（软件 I2C） | ⬜ 待实现 | 见下方接线，尚未编写驱动 |
| 平衡控制 / PID | ⬜ 待实现 | 核心算法尚未加入 |

---

## 6. 硬件接线（官方 B570 参考）

所有引脚均焊接在 PCB 上，不可杜邦线变通。注意 **PB3 / PB4 / PA15 是 JTAG 复用脚**，已在 `HAL_MspInit` 中用 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 释放为 GPIO，用于 OLED；不要改回全 JTAG，否则 OLED 失效。

| 功能 | 引脚 | 备注 |
|------|------|------|
| OLED RES | PB3 | 软件 SPI |
| OLED DC (RS) | PA15 | 软件 SPI |
| OLED SCL | PB5 | 软件 SPI |
| OLED SDIN | PB4 | 软件 SPI |
| 编码器 L | PA0=TIM2_CH1, PA1=TIM2_CH2 | |
| 编码器 R | PB6=TIM4_CH1, PB7=TIM4_CH2 | |
| 电机 PWM L | PA8=TIM1_CH1 | |
| 电机 PWM R | PA11=TIM1_CH4 | |
| 电机方向 L | PB14=AIN1, PB15=AIN2 | |
| 电机方向 R | PB13=BIN1, PB12=BIN2 | |
| MPU6050 INT | PA12=EXTI | |
| MPU6050 I2C | PB8=SCL, PB9=SDA | 软件 I2C |
| LED | PA4 | 低电平点亮 |
| 按键 | PA5 | |
| 电池电压 ADC | PA6 | |
| 蓝牙 USART3 | PB10=TX, PB11=RX | DMA 空闲中断 |

---

## 7. 已知注意事项

1. **OLED 必须手动刷新**：驱动是显存缓冲，`ShowString` 等只写 RAM，不调用 `OLED_Refresh_Gram()` 屏会一直全黑（看起来"不亮"）。
2. **JTAG 脚复用**：PB3/PB4/PA15 已通过 `SWJ_NOJTAG` 释放给 OLED，保持该配置。
3. **无线调试器无 nRST**：`openocd.cfg` 用 `reset_config none`，复位走 SYSRESETREQ；如需硬复位需另外接 nRST。
4. **芯片型号**：当前固件按 STM32F103C8（64 KB）调试。若板子为兼容/打磨片（如 air32f103），OpenOCD 用 `target/stm32f1x.cfg` 一般可识别；换 pyOCD 时可能需要指定对应 target。

---

## 8. 后续待办

- [ ] 编码器计数（TIMx 编码器模式）
- [ ] 电机 PID 与 PWM 输出
- [ ] MPU6050 读取 + 卡尔曼/互补滤波
- [ ] 直立平衡控制闭环
- [ ] 蓝牙指令/遥测
