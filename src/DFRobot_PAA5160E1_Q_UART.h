/*!
 * @file DFRobot_PAA5160E1_Q_UART.h
 * @brief Fermion: PAA5160-E1 光学里程计的 UART 接口（MODBUS RTU）。
 * @details 模块 UART 模式使用 MODBUS RTU（DFRobot General RTU v1.0）：默认从站地址 0x32、9600 8N1。
 *          CRC16、请求组帧、响应解析和超时循环全部复用 DFRobot_RTU 库（协议绑定
 *          dfrobot-rtu-arduino-client，depends=DFRobot_RTU），本类只负责产品寄存器常量、
 *          数值编解码和产品 API。
 * @n 模块的 I2C 与 UART 由 mode 引脚在上电时决定，本类只适用于 UART 模式。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */

#ifndef __DFROBOT_PAA5160E1_Q_UART_H  ///< 头文件包含保护
#define __DFROBOT_PAA5160E1_Q_UART_H  ///< 头文件包含保护

#include "Arduino.h"
#include "DFRobot_RTU.h"
#include "DFRobot_PAA5160E1_Q.h"

/* 只有带硬件 Serial1 的板子（Mega2560、ESP32 等）才能给默认串口；
 * UNO/Nano 这类没有 Serial1 的板子默认传 NULL，必须由用户显式传入串口对象，
 * 否则库在 UNO 上无法编译。 */
#if defined(HAVE_HWSERIAL1) || defined(SERIAL_1_AVAILABLE) || defined(ARDUINO_ARCH_ESP32) || \
    defined(ARDUINO_ARCH_RP2040)
#define PAA5160_UART_DEFAULT_SERIAL &Serial1  ///< 有 Serial1 的板子默认用它
#else
#define PAA5160_UART_DEFAULT_SERIAL NULL  ///< 没有 Serial1 的板子必须显式传入
#endif

/*!
 * @class DFRobot_PAA5160E1_Q_UART
 * @brief UART（MODBUS RTU）模式下的光学里程计对象，继承 DFRobot_RTU 复用其传输层。
 */
class DFRobot_PAA5160E1_Q_UART : public DFRobot_RTU {
public:
  /*!
   * @fn DFRobot_PAA5160E1_Q_UART
   * @brief UART 接口构造函数，绑定硬件串口对象。
   * @param serial UART 串口对象（HardwareSerial）。带 Serial1 的板子（Mega2560/ESP32/RP2040）默认 &Serial1；
   *        UNO/Nano 这类没有 Serial1 的板子默认 NULL，必须由用户显式传入串口对象。
   * @param baudRate 串口波特率，固定为 9600，仅做兼容传参
   * @n 串口固定参数：8 数据位、无校验、1 停止位、无硬件流控（9600 8N1）；传输层由 DFRobot_RTU 提供。
   * @return 无
   */
  DFRobot_PAA5160E1_Q_UART(HardwareSerial *serial = PAA5160_UART_DEFAULT_SERIAL,
                           uint32_t baudRate = 9600);

  /*!
   * @fn begin
   * @brief 打开串口并按产品协议读取 PID，确认通讯正常。
   * @return bool true=识别到模块；false=无应答或身份不符
   * @n 1. 本接口只适用于模块上电时为 UART 模式的情况；I2C 模式请用 DFRobot_PAA5160E1_Q_I2C；
   * @n 2. 波特率固定 9600，构造函数里的 baudRate 只是兼容传参。
   */
  bool begin(void);

  /*!
   * @fn setSlaveAddress
   * @brief 设置本库使用的 MODBUS 从站地址。
   * @param addr 从站地址，1~247
   * @return bool true=参数合法并已设置
   * @n 模块出厂默认从站地址为 0x32；本接口只改主机侧使用的地址，不改模块内部地址。
   */
  bool setSlaveAddress(uint8_t addr);

  /*!
   * @fn getSlaveAddress
   * @brief 读取本库当前使用的 MODBUS 从站地址。
   * @return 从站地址
   */
  uint8_t getSlaveAddress(void);

  /*!
   * @fn getPAA_ID
   * @brief 读取 PAA5160E1-Q 芯片产品号。
   * @param productId 输出 2 字节产品号
   * @return bool true=读取成功
   * @n 该值来自输入寄存器 0x0006（1 个寄存器，2 字节），模块识别不到芯片时读回 0。
   */
  bool getPAA_ID(uint16_t &productId);

  /*!
   * @fn getPAA_RevisionID
   * @brief 读取 PAA5160E1-Q 芯片固件版本。
   * @param firmwareVer 输出 3 字节版本号
   * @return bool true=读取成功
   * @n 该值来自输入寄存器 0x0007~0x0008（2 个寄存器，4 字节），只取前 3 字节。
   */
  bool getPAA_RevisionID(uint8_t firmwareVer[3]);

  /*!
   * @fn getTotalDisplacement
   * @brief 获取软件侧持续累加的 X/Y 总位移。
   * @param totalX_cm 输出 X 轴总位移，单位 cm
   * @param totalY_cm 输出 Y 轴总位移，单位 cm
   * @return int PAA5160_OK=获取成功；PAA5160_ERR_IO=通讯失败
   * @n 1. 一次读输入寄存器 0x0009~0x000C（4 个寄存器），X/Y 各为 IEEE-754 单精度大端；
   * @n 2. 模块不断电时数值持续累加，断电后累加值丢失；本接口不清零累加器。
   */
  int getTotalDisplacement(float *totalX_cm, float *totalY_cm);

  /*!
   * @fn resetTotalDisplacement
   * @brief 清零模块软件侧累加的总位移。
   * @return int PAA5160_OK=清零成功；PAA5160_ERR_IO=通讯失败
   * @n 写保持寄存器 0x000C（CLEAR_DISPLACEMENT），写任意值触发，校准系数不受影响。
   */
  int resetTotalDisplacement(void);

  /*!
   * @fn setCalibrationScale
   * @brief 设置 X/Y 位移校准比例系数，掉电保存。
   * @param scaleX X 轴校准比例
   * @param scaleY Y 轴校准比例
   * @return int PAA5160_OK=保存成功；PAA5160_ERR_PARAM=系数超出范围；PAA5160_ERR_IO=通讯失败
   * @n 1. 系数含义为「实际距离 / 模块读数」，例如实际 100 cm、模块读数 109 cm，则系数取 100.0/109.0；
   * @n 2. 建议范围 0.5 ~ 2.0，超出说明校准距离不足，本接口返回参数错误；
   * @n 3. 分别写入保持寄存器 0x0007~0x0008 与 0x0009~0x000A，各 2 个寄存器（float32 大端）。
   */
  int setCalibrationScale(float scaleX, float scaleY);

  /*!
   * @fn setCpi
   * @brief 设置光流传感器 CPI 分辨率，掉电保存。
   * @param cpi CPI 挡位，取值 1~200，共 200 档
   * @return int PAA5160_OK=设置成功；PAA5160_ERR_PARAM=参数非法；PAA5160_ERR_IO=通讯失败
   * @n 1. 写保持寄存器 0x0006，模块侧只对后续新读取的位移增量生效；
   * @n 2. 高 CPI 分辨好但噪声大、易溢出；低 CPI 噪声小但微小位移会丢失。
   */
  int setCpi(uint16_t cpi);

  /*!
   * @fn readSqual
   * @brief 读取 SQUAL 表面特征质量。
   * @param squal 输出原始寄存器值 0~255
   * @return int PAA5160_OK=读取成功；PAA5160_ERR_PARAM=空指针；PAA5160_ERR_IO=通讯失败
   * @n 有效特征点数量 = squal × 4；值偏低表示表面特征不足，位移可信度下降。
   */
  int readSqual(uint8_t *squal);

  /*!
   * @fn setOrientation
   * @brief 设置光流坐标系方向，不掉电保存。
   * @param swapXY 1=交换 X、Y 轴；0=不交换
   * @param invertX 1=翻转 X 轴方向；0=不翻转
   * @param invertY 1=翻转 Y 轴方向；0=不翻转
   * @return bool true=配置成功
   * @n 1. 各参数可选值：0 = 不处理，1 = 启用对应变换；
   * @n 2. 同时开启交换与翻转时，模块先执行 XY 交换、再执行方向翻转；
   * @n 3. 写保持寄存器 0x000B，配置不掉电保存。
   */
  bool setOrientation(bool swapXY, bool invertX, bool invertY);

private:
  HardwareSerial *_serial;
  uint32_t _baudRate;
  uint8_t _slaveAddr;
};

#endif
