/*!
 * @file DFRobot_PAA5160E1_Q_I2C.h
 * @brief Fermion: PAA5160-E1 光学里程计的 I2C 接口。
 * @details 模块内部由板载 MCU 读取 PAA5160E1-Q 的位移增量并累加；主机通过 I2C 读取模块对外寄存器。
 *          寄存器映射见 requirements.md §3.4：单字节寄存器地址、读写后地址自动加 1、多字节字段为大端
 *          IEEE-754 单精度，单位 cm。
 * @n I2C 与 UART 是同一模块的两种模式，由上电时 mode 引脚决定，同一时刻只有一种可用，本类只适用于 I2C 模式。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */

#ifndef __DFROBOT_PAA5160E1_Q_I2C_H  ///< 头文件包含保护
#define __DFROBOT_PAA5160E1_Q_I2C_H  ///< 头文件包含保护

#include "Arduino.h"
#include "Wire.h"
#include "DFRobot_PAA5160E1_Q.h"

/*!
 * @class DFRobot_PAA5160E1_Q_I2C
 * @brief I2C 模式下的光学里程计对象。
 */
class DFRobot_PAA5160E1_Q_I2C {
public:
  /*!
   * @fn DFRobot_PAA5160E1_Q_I2C
   * @brief 构造函数，绑定 I2C 总线与模块从机地址。
   * @param pWire I2C 总线对象
   * @param i2cAddr 模块从机地址，默认 0x11（模块 addr 脚外部下拉）
   * @n 模块 addr 脚上拉时地址为 0x12；地址在上电时采样，运行中不可改。
   * @return 无
   */
  DFRobot_PAA5160E1_Q_I2C(TwoWire *pWire = &Wire, uint8_t i2cAddr = 0x11);

  /*!
   * @fn begin
   * @brief I2C 初始化并读取模块身份寄存器，确认通讯正常。
   * @return bool true=识别到模块；false=无设备或通讯失败
   * @n 本接口只适用于模块上电时为 I2C 模式的情况；UART 模式请用 DFRobot_PAA5160E1_Q_UART。
   */
  bool begin(void);

  /*!
   * @fn getDeviceAddress
   * @brief 读取当前使用的 I2C 从机地址。
   * @return 从机地址（0x11 或 0x12）
   */
  uint8_t getDeviceAddress(void);

  /*!
   * @fn getPAA_ID
   * @brief 读取 PAA5160E1-Q 芯片产品号。
   * @param productId 输出 2 字节产品号
   * @return bool true=读取成功
   * @n 该值来自模块对外寄存器 0x10（2 字节），模块识别不到芯片时读回全 0。
   */
  bool getPAA_ID(uint16_t &productId);

  /*!
   * @fn getPAA_RevisionID
   * @brief 读取 PAA5160E1-Q 芯片固件版本。
   * @param firmwareVer 输出 3 字节版本号
   * @return bool true=读取成功
   * @n 该值来自模块对外寄存器 0x12（3 字节）；模块识别不到芯片时读回全 0。
   */
  bool getPAA_RevisionID(uint8_t firmwareVer[3]);

  /*!
   * @fn getTotalDisplacement
   * @brief 获取软件侧持续累加的 X/Y 总位移。
   * @param totalX_cm 输出 X 轴总位移，单位 cm
   * @param totalY_cm 输出 Y 轴总位移，单位 cm
   * @return int PAA5160_OK=获取成功；PAA5160_ERR_IO=通讯失败
   * @n 1. 传感器不断电时数值持续累加，断电后累加值丢失；
   * @n 2. 本接口不会清零内部累加器；模块内部自动读取 Δcount、按 CPI 换算并应用校准系数。
   */
  int getTotalDisplacement(float *totalX_cm, float *totalY_cm);

  /*!
   * @fn resetTotalDisplacement
   * @brief 清零模块软件侧累加的总位移，把 totalX、totalY 归 0。
   * @return int PAA5160_OK=清零成功；PAA5160_ERR_IO=通讯失败
   * @n 只清模块的 XY 位移累加，校准系数不受影响。
   */
  int resetTotalDisplacement(void);

  /*!
   * @fn setCalibrationScale
   * @brief 设置 X/Y 位移校准比例系数，掉电保存。
   * @param scaleX X 轴校准比例
   * @param scaleY Y 轴校准比例
   * @return int PAA5160_OK=保存成功；PAA5160_ERR_PARAM=系数超出范围；PAA5160_ERR_IO=通讯失败
   * @n 1. 系数含义为「实际距离 / 模块读数」，例如实际移动 100 cm、模块输出 109 cm，则系数取 100.0/109.0；
   * @n 2. 建议范围 0.5 ~ 2.0，超出说明校准距离不足，本接口返回参数错误，请适当增加校准距离；
   * @n 3. 写入后存入模块 Flash，模块重启或断电重上电自动加载生效。
   */
  int setCalibrationScale(float scaleX, float scaleY);

  /*!
   * @fn setCpi
   * @brief 设置光流传感器 CPI（Counts-Per-Inch）分辨率，掉电保存。
   * @param cpi CPI 挡位，取值 1~200，共 200 档
   * @return int PAA5160_OK=设置成功；PAA5160_ERR_PARAM=参数非法；PAA5160_ERR_IO=通讯失败
   * @n 1. 修改 CPI 只对后续新读取的位移增量生效，已经累加的总位移不会重算；
   * @n 2. 高 CPI：微小位移分辨好，噪声更大、更容易发生数据溢出，需要更高的读取速率；
   * @n 3. 低 CPI：噪声小、支持更高运动速度，但微小位移会丢失。
   */
  int setCpi(uint16_t cpi);

  /*!
   * @fn readSqual
   * @brief 读取 SQUAL 表面特征质量，用于判断当前表面对光流跟踪是否可信。
   * @param squal 输出原始寄存器值 0~255
   * @return int PAA5160_OK=读取成功；PAA5160_ERR_PARAM=空指针；PAA5160_ERR_IO=通讯失败
   * @n 有效特征点数量 = squal × 4；值偏低表示表面特征不足，位移可信度下降。
   */
  int readSqual(uint8_t *squal);

  /*!
   * @fn setOrientation
   * @brief 设置光流坐标系方向（交换 XY、翻转 X/Y 方向），不掉电保存。
   * @param swapXY 1=交换 X、Y 轴；0=不交换
   * @param invertX 1=翻转 X 轴方向；0=不翻转
   * @param invertY 1=翻转 Y 轴方向；0=不翻转
   * @return bool true=配置成功
   * @n 1. 各参数可选值：0 = 不处理，1 = 启用对应变换；
   * @n 2. 同时开启交换和翻转时，模块先执行 XY 交换、再执行方向翻转；
   * @n 3. 配置不掉电保存，重新上电后回到默认方向。
   */
  bool setOrientation(bool swapXY, bool invertX, bool invertY);

private:
  int readRegisters(uint8_t reg, uint8_t *data, uint8_t len);
  int writeRegisters(uint8_t reg, const uint8_t *data, uint8_t len);
  int writeU16(uint8_t reg, uint16_t value);
  int writeFloat(uint8_t reg, float value);
  int readFloat(uint8_t reg, float *value);

  TwoWire *_pWire;
  uint8_t _i2cAddr;
};

#endif
