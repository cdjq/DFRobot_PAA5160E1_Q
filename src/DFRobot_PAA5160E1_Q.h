/*!
 * @file DFRobot_PAA5160E1_Q.h
 * @brief Fermion: PAA5160-E1 光学里程计的公共定义（I2C / UART 两个类共用）。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */

#ifndef __DFROBOT_PAA5160E1_Q_H  ///< 头文件包含保护
#define __DFROBOT_PAA5160E1_Q_H  ///< 头文件包含保护

#include <Arduino.h>

/** @brief 返回码：成功 */
#define PAA5160_OK 0  ///< 返回码：成功
/** @brief 返回码：参数非法（例如 cpi 超出 1~200、校准系数超出 0.5~2.0） */
#define PAA5160_ERR_PARAM (-1)  ///< 返回码：参数非法
/** @brief 返回码：通讯失败或模块无应答 */
#define PAA5160_ERR_IO (-2)  ///< 返回码：通讯失败

/* ---------------- I2C 模式寄存器映射（requirements.md §3.4） ----------------
 * 单字节寄存器地址，读写后地址自动加 1，多字节字段为大端 IEEE-754 单精度。
 * 这些地址全部避开 PAA5160E1-Q 芯片的原生寄存器地址。
 */
#define PAA5160_I2C_REG_PAA_ID 0x10        ///< 芯片产品号，2 字节（Product_ID + Inverse_Product_ID）
#define PAA5160_I2C_REG_PAA_REVISION 0x12  ///< 芯片版本，3 字节
#define PAA5160_I2C_REG_TOTAL_X 0x20       ///< X 轴累计位移，IEEE-754 单精度大端，单位 cm
#define PAA5160_I2C_REG_TOTAL_Y 0x24       ///< Y 轴累计位移，IEEE-754 单精度大端，单位 cm
#define PAA5160_I2C_REG_SQUAL 0x28         ///< 表面特征质量，1 字节，有效特征点 = 值 × 4
#define PAA5160_I2C_REG_CPI 0x30           ///< CPI 挡位，2 字节，取值 1~200
#define PAA5160_I2C_REG_CAL_SCALE_X 0x32   ///< X 校准系数，IEEE-754 单精度大端
#define PAA5160_I2C_REG_CAL_SCALE_Y 0x36   ///< Y 校准系数，IEEE-754 单精度大端
#define PAA5160_I2C_REG_ORIENTATION 0x40   ///< 方向位：bit0 交换 XY、bit1 翻转 X、bit2 翻转 Y
#define PAA5160_I2C_REG_CLEAR 0x41         ///< 写任意值清零累计位移

/* ---------------- UART 模式寄存器（requirements.md §3.1~§3.3） ----------------
 * MODBUS RTU，默认从站地址 0x32、9600 8N1，寄存器值线上大端。
 */
#define PAA5160_UART_INPUT_PID 0x0000      ///< 产品 PID（只读）
#define PAA5160_UART_INPUT_VID 0x0001      ///< DFRobot 厂商标识 0x3343（只读）
#define PAA5160_UART_INPUT_ADDR 0x0002     ///< 当前从站地址（只读）
#define PAA5160_UART_INPUT_VERSION 0x0005  ///< 固件版本（只读）
#define PAA5160_UART_INPUT_PAA_ID 0x0006   ///< 芯片产品号 2 字节（只读）
#define PAA5160_UART_INPUT_PAA_REV 0x0007  ///< 芯片版本 3 字节，占 0x0007~0x0008（只读）
#define PAA5160_UART_INPUT_TOTAL_X 0x0009  ///< X 轴累计位移 float32，占 0x0009~0x000A，单位 cm（只读）
#define PAA5160_UART_INPUT_TOTAL_Y 0x000B  ///< Y 轴累计位移 float32，占 0x000B~0x000C，单位 cm（只读）
#define PAA5160_UART_INPUT_SQUAL 0x000D    ///< 表面特征质量（只读）

#define PAA5160_UART_HOLD_CPI 0x0006           ///< CPI 挡位 1~200（读写）
#define PAA5160_UART_HOLD_CAL_SCALE_X 0x0007   ///< X 校准系数 float32，占 0x0007~0x0008（读写）
#define PAA5160_UART_HOLD_CAL_SCALE_Y 0x0009   ///< Y 校准系数 float32，占 0x0009~0x000A（读写）
#define PAA5160_UART_HOLD_ORIENTATION 0x000B   ///< 方向位：bit0 交换 XY、bit1 翻转 X、bit2 翻转 Y（读写）
#define PAA5160_UART_HOLD_CLEAR 0x000C         ///< 写任意值清零累计位移（只写）

/** @brief 产品对外标识（requirements.md §3.6 的本次规格定义值） */
#define PAA5160_PRODUCT_PID 0x0777  ///< MODBUS 输入寄存器 0x0000 的产品 PID
#define PAA5160_PRODUCT_VID 0x3343  ///< MODBUS 输入寄存器 0x0001 的 DFRobot VID

/**
 * @enum ePAA5160Mode_t
 * @brief 模块的对外接口模式。上电时由模块的 mode 引脚决定，运行中不可切换。
 */
typedef enum {
  ePAA5160ModeI2C = 0,  /**< I2C 模式（mode 脚外部下拉，默认） */
  ePAA5160ModeUART = 1, /**< UART 模式（mode 脚上拉），MODBUS RTU 9600 8N1 */
} ePAA5160Mode_t;

#endif
