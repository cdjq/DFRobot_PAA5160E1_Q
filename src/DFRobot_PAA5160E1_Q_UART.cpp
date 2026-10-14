/*!
 * @file DFRobot_PAA5160E1_Q_UART.cpp
 * @brief Fermion: PAA5160-E1 光学里程计的 UART（MODBUS RTU）接口实现。
 * @details 传输层（CRC16、请求组帧、响应解析、超时循环）全部复用 DFRobot_RTU 库；
 *          本文件只做产品寄存器的数值编解码与产品 API。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */

#include "DFRobot_PAA5160E1_Q_UART.h"

DFRobot_PAA5160E1_Q_UART::DFRobot_PAA5160E1_Q_UART(HardwareSerial *serial, uint32_t baudRate)
    : DFRobot_RTU(serial)
{
  _serial = serial;
  _baudRate = baudRate;
  _slaveAddr = 0x32;
}

bool DFRobot_PAA5160E1_Q_UART::begin(void)
{
  uint16_t regs[2] = {0};
  uint8_t attempt;

  if (_serial == NULL) {
    return false;
  }
  _serial->begin(_baudRate);
  /* 模块内部要读传感器再应答，超时给足；传输层由 DFRobot_RTU 处理。 */
  setTimeoutTimeMs(300);
  delay(50);
  /* 身份读的是产品 PID/VID（输入寄存器 0x0000/0x0001），不是 PAA 芯片号（0x0006），
   * 两者含义不同，不能混用。
   * 模块上电后会先初始化光流传感器（约 1~2 s），这段时间它不解析主机帧，所以重试几次。 */
  for (attempt = 0; attempt < 10u; attempt++) {
    regs[0] = 0;
    regs[1] = 0;
    if (readInputRegister(_slaveAddr, PAA5160_UART_INPUT_PID, regs, 2) == 0 &&
        regs[0] == PAA5160_PRODUCT_PID) {
      return true;
    }
    delay(200);
  }
  return false;
}

bool DFRobot_PAA5160E1_Q_UART::setSlaveAddress(uint8_t addr)
{
  if (addr < 1 || addr > 247) {
    return false;
  }
  _slaveAddr = addr;
  return true;
}

uint8_t DFRobot_PAA5160E1_Q_UART::getSlaveAddress(void)
{
  return _slaveAddr;
}

bool DFRobot_PAA5160E1_Q_UART::getPAA_ID(uint16_t &productId)
{
  uint16_t regs[1] = {0};

  if (readInputRegister(_slaveAddr, PAA5160_UART_INPUT_PAA_ID, regs, 1) != 0) {
    return false;
  }
  productId = regs[0];
  return true;
}

bool DFRobot_PAA5160E1_Q_UART::getPAA_RevisionID(uint8_t firmwareVer[3])
{
  uint16_t regs[2] = {0};

  if (firmwareVer == NULL) {
    return false;
  }
  if (readInputRegister(_slaveAddr, PAA5160_UART_INPUT_PAA_REV, regs, 2) != 0) {
    return false;
  }
  /* 3 字节版本占 2 个寄存器：b0 b1 在第一个寄存器，b2 在第二个寄存器的高字节。 */
  firmwareVer[0] = (uint8_t)(regs[0] >> 8);
  firmwareVer[1] = (uint8_t)(regs[0] & 0xFF);
  firmwareVer[2] = (uint8_t)(regs[1] >> 8);
  return true;
}

int DFRobot_PAA5160E1_Q_UART::getTotalDisplacement(float *totalX_cm, float *totalY_cm)
{
  uint16_t regs[4] = {0};
  uint32_t bits;

  if (totalX_cm == NULL || totalY_cm == NULL) {
    return PAA5160_ERR_PARAM;
  }
  if (readInputRegister(_slaveAddr, PAA5160_UART_INPUT_TOTAL_X, regs, 4) != 0) {
    return PAA5160_ERR_IO;
  }
  /* 两个 float32（IEEE-754 单精度大端）各占 2 个寄存器。 */
  bits = ((uint32_t)regs[0] << 16) | (uint32_t)regs[1];
  memcpy(totalX_cm, &bits, sizeof(*totalX_cm));
  bits = ((uint32_t)regs[2] << 16) | (uint32_t)regs[3];
  memcpy(totalY_cm, &bits, sizeof(*totalY_cm));
  return PAA5160_OK;
}

int DFRobot_PAA5160E1_Q_UART::resetTotalDisplacement(void)
{
  return (writeHoldingRegister(_slaveAddr, PAA5160_UART_HOLD_CLEAR, 0x0001) == 0) ? PAA5160_OK
                                                                                : PAA5160_ERR_IO;
}

int DFRobot_PAA5160E1_Q_UART::setCalibrationScale(float scaleX, float scaleY)
{
  uint16_t regs[2];
  uint32_t bits;

  if (scaleX < 0.5f || scaleX > 2.0f || scaleY < 0.5f || scaleY > 2.0f) {
    return PAA5160_ERR_PARAM;
  }
  memcpy(&bits, &scaleX, sizeof(bits));
  regs[0] = (uint16_t)(bits >> 16);
  regs[1] = (uint16_t)(bits & 0xFFFF);
  if (writeHoldingRegister(_slaveAddr, PAA5160_UART_HOLD_CAL_SCALE_X, regs, 2) != 0) {
    return PAA5160_ERR_IO;
  }
  memcpy(&bits, &scaleY, sizeof(bits));
  regs[0] = (uint16_t)(bits >> 16);
  regs[1] = (uint16_t)(bits & 0xFFFF);
  if (writeHoldingRegister(_slaveAddr, PAA5160_UART_HOLD_CAL_SCALE_Y, regs, 2) != 0) {
    return PAA5160_ERR_IO;
  }
  return PAA5160_OK;
}

int DFRobot_PAA5160E1_Q_UART::setCpi(uint16_t cpi)
{
  if (cpi < 1 || cpi > 200) {
    return PAA5160_ERR_PARAM;
  }
  return (writeHoldingRegister(_slaveAddr, PAA5160_UART_HOLD_CPI, cpi) == 0) ? PAA5160_OK
                                                                            : PAA5160_ERR_IO;
}

int DFRobot_PAA5160E1_Q_UART::readSqual(uint8_t *squal)
{
  uint16_t regs[1] = {0};

  if (squal == NULL) {
    return PAA5160_ERR_PARAM;
  }
  if (readInputRegister(_slaveAddr, PAA5160_UART_INPUT_SQUAL, regs, 1) != 0) {
    return PAA5160_ERR_IO;
  }
  *squal = (uint8_t)(regs[0] & 0xFF);
  return PAA5160_OK;
}

bool DFRobot_PAA5160E1_Q_UART::setOrientation(bool swapXY, bool invertX, bool invertY)
{
  uint16_t value = 0;

  if (swapXY) {
    value |= 0x0001;
  }
  if (invertX) {
    value |= 0x0002;
  }
  if (invertY) {
    value |= 0x0004;
  }
  return writeHoldingRegister(_slaveAddr, PAA5160_UART_HOLD_ORIENTATION, value) == 0;
}
