/*!
 * @file DFRobot_PAA5160E1_Q_I2C.cpp
 * @brief Fermion: PAA5160-E1 光学里程计的 I2C 接口实现。
 * @details 只按 requirements.md §3.4 的寄存器映射读写模块：单字节寄存器地址 + 自动递增，
 *          多字节字段为大端 IEEE-754 单精度。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */

#include "DFRobot_PAA5160E1_Q_I2C.h"

DFRobot_PAA5160E1_Q_I2C::DFRobot_PAA5160E1_Q_I2C(TwoWire *pWire, uint8_t i2cAddr)
{
  _pWire = pWire;
  _i2cAddr = i2cAddr;
}

bool DFRobot_PAA5160E1_Q_I2C::begin(void)
{
  uint16_t productId = 0;

  if (_pWire == NULL) {
    return false;
  }
  _pWire->begin();
  delay(50);
  /* 读一次芯片产品号：读得回非 0/非 0xFFFF 就认为模块在总线上。 */
  if (!getPAA_ID(productId)) {
    return false;
  }
  return (productId != 0) && (productId != 0xFFFF);
}

uint8_t DFRobot_PAA5160E1_Q_I2C::getDeviceAddress(void)
{
  return _i2cAddr;
}

int DFRobot_PAA5160E1_Q_I2C::readRegisters(uint8_t reg, uint8_t *data, uint8_t len)
{
  uint8_t i;

  if (_pWire == NULL || data == NULL || len == 0) {
    return PAA5160_ERR_PARAM;
  }
  _pWire->beginTransmission(_i2cAddr);
  _pWire->write(reg);
  /* 用重复起始读，保证寄存器指针在同一事务里生效。 */
  if (_pWire->endTransmission(false) != 0) {
    return PAA5160_ERR_IO;
  }
  if (_pWire->requestFrom((uint8_t)_i2cAddr, len) != len) {
    return PAA5160_ERR_IO;
  }
  for (i = 0; i < len; i++) {
    data[i] = (uint8_t)_pWire->read();
  }
  return PAA5160_OK;
}

int DFRobot_PAA5160E1_Q_I2C::writeRegisters(uint8_t reg, const uint8_t *data, uint8_t len)
{
  uint8_t i;

  if (_pWire == NULL || data == NULL || len == 0) {
    return PAA5160_ERR_PARAM;
  }
  _pWire->beginTransmission(_i2cAddr);
  _pWire->write(reg);
  for (i = 0; i < len; i++) {
    _pWire->write(data[i]);
  }
  if (_pWire->endTransmission() != 0) {
    return PAA5160_ERR_IO;
  }
  /* 从机在中断里只把写排队，真正生效在主循环；等一拍再回读更稳。 */
  delay(50);
  return PAA5160_OK;
}

int DFRobot_PAA5160E1_Q_I2C::writeU16(uint8_t reg, uint16_t value)
{
  uint8_t buf[2];

  buf[0] = (uint8_t)(value >> 8);
  buf[1] = (uint8_t)(value & 0xFF);
  return writeRegisters(reg, buf, 2);
}

int DFRobot_PAA5160E1_Q_I2C::writeFloat(uint8_t reg, float value)
{
  uint8_t buf[4];
  uint32_t bits = 0;

  memcpy(&bits, &value, sizeof(bits));
  buf[0] = (uint8_t)(bits >> 24);
  buf[1] = (uint8_t)(bits >> 16);
  buf[2] = (uint8_t)(bits >> 8);
  buf[3] = (uint8_t)(bits & 0xFF);
  return writeRegisters(reg, buf, 4);
}

int DFRobot_PAA5160E1_Q_I2C::readFloat(uint8_t reg, float *value)
{
  uint8_t buf[4];
  uint32_t bits = 0;
  int ret;

  if (value == NULL) {
    return PAA5160_ERR_PARAM;
  }
  ret = readRegisters(reg, buf, 4);
  if (ret != PAA5160_OK) {
    return ret;
  }
  bits = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) |
         (uint32_t)buf[3];
  memcpy(value, &bits, sizeof(*value));
  return PAA5160_OK;
}

bool DFRobot_PAA5160E1_Q_I2C::getPAA_ID(uint16_t &productId)
{
  uint8_t buf[2];

  if (readRegisters(PAA5160_I2C_REG_PAA_ID, buf, 2) != PAA5160_OK) {
    return false;
  }
  productId = ((uint16_t)buf[0] << 8) | buf[1];
  return true;
}

bool DFRobot_PAA5160E1_Q_I2C::getPAA_RevisionID(uint8_t firmwareVer[3])
{
  if (firmwareVer == NULL) {
    return false;
  }
  return readRegisters(PAA5160_I2C_REG_PAA_REVISION, firmwareVer, 3) == PAA5160_OK;
}

int DFRobot_PAA5160E1_Q_I2C::getTotalDisplacement(float *totalX_cm, float *totalY_cm)
{
  int ret;

  if (totalX_cm == NULL || totalY_cm == NULL) {
    return PAA5160_ERR_PARAM;
  }
  ret = readFloat(PAA5160_I2C_REG_TOTAL_X, totalX_cm);
  if (ret != PAA5160_OK) {
    return ret;
  }
  return readFloat(PAA5160_I2C_REG_TOTAL_Y, totalY_cm);
}

int DFRobot_PAA5160E1_Q_I2C::resetTotalDisplacement(void)
{
  uint8_t value = 0x01;

  return writeRegisters(PAA5160_I2C_REG_CLEAR, &value, 1);
}

int DFRobot_PAA5160E1_Q_I2C::setCalibrationScale(float scaleX, float scaleY)
{
  int ret;

  if (scaleX < 0.5f || scaleX > 2.0f || scaleY < 0.5f || scaleY > 2.0f) {
    return PAA5160_ERR_PARAM;
  }
  ret = writeFloat(PAA5160_I2C_REG_CAL_SCALE_X, scaleX);
  if (ret != PAA5160_OK) {
    return ret;
  }
  return writeFloat(PAA5160_I2C_REG_CAL_SCALE_Y, scaleY);
}

int DFRobot_PAA5160E1_Q_I2C::setCpi(uint16_t cpi)
{
  if (cpi < 1 || cpi > 200) {
    return PAA5160_ERR_PARAM;
  }
  return writeU16(PAA5160_I2C_REG_CPI, cpi);
}

int DFRobot_PAA5160E1_Q_I2C::readSqual(uint8_t *squal)
{
  if (squal == NULL) {
    return PAA5160_ERR_PARAM;
  }
  return readRegisters(PAA5160_I2C_REG_SQUAL, squal, 1);
}

bool DFRobot_PAA5160E1_Q_I2C::setOrientation(bool swapXY, bool invertX, bool invertY)
{
  uint8_t value = 0;

  if (swapXY) {
    value |= 0x01;
  }
  if (invertX) {
    value |= 0x02;
  }
  if (invertY) {
    value |= 0x04;
  }
  return writeRegisters(PAA5160_I2C_REG_ORIENTATION, &value, 1) == PAA5160_OK;
}
