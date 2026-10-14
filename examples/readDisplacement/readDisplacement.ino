/*!
 * @file readDisplacement.ino
 * @brief 主示例（I2C）：读取 Fermion: PAA5160-E1 光学里程计的身份、表面质量与 X/Y 总位移。
 * @details setup 里完成完整配置链：begin() → 打印芯片产品号/版本 → setCpi(20) → setOrientation(0,0,0)
 *          → resetTotalDisplacement()；loop 里周期打印 SQUAL 与 X/Y 总位移（cm）。
 * @n 接线（I2C 模式，模块 mode 脚保持外部下拉的默认状态）：
 * @n UNO     : SDA=A4, SCL=A5, VCC=3.3V 或 5V, GND=GND
 * @n ESP32   : SDA=GPIO21, SCL=GPIO22, VCC=3.3V, GND=GND（Wire 默认脚）
 * @n Mega2560: SDA=20,  SCL=21,  VCC=3.3V 或 5V, GND=GND
 * @n 模块 I/O 为 3.3 V 电平，用 5 V 主机时请确认上拉电平或加电平转换。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */
#include "DFRobot_PAA5160E1_Q_I2C.h"

/* 模块默认 I2C 地址 0x11（addr 脚下拉）；addr 脚上拉时为 0x12。 */
DFRobot_PAA5160E1_Q_I2C odometry(&Wire, 0x11);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Fermion PAA5160-E1 optical odometry (I2C)");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (I2C).");
    delay(1000);
  }

  uint16_t productId = 0;
  uint8_t revision[3] = {0};
  if (odometry.getPAA_ID(productId)) {
    Serial.print("PAA5160E1-Q product id: 0x");
    Serial.println(productId, HEX);
  }
  if (odometry.getPAA_RevisionID(revision)) {
    Serial.print("PAA5160E1-Q revision  : 0x");
    Serial.print(revision[0], HEX);
    Serial.print(revision[1], HEX);
    Serial.println(revision[2], HEX);
  }

  /* 配置链：先设 CPI 挡位（20 = 2000 cpi），再把方向恢复为默认，最后清零累加位移。 */
  if (odometry.setCpi(20) != PAA5160_OK) {
    Serial.println("setCpi failed.");
  }
  if (!odometry.setOrientation(false, false, false)) {
    Serial.println("setOrientation failed.");
  }
  if (odometry.resetTotalDisplacement() != PAA5160_OK) {
    Serial.println("resetTotalDisplacement failed.");
  }
}

void loop()
{
  float totalX = 0.0f;
  float totalY = 0.0f;
  uint8_t squal = 0;

  if (odometry.getTotalDisplacement(&totalX, &totalY) == PAA5160_OK) {
    Serial.print("total displacement  X = ");
    Serial.print(totalX, 3);
    Serial.print(" cm, Y = ");
    Serial.print(totalY, 3);
    Serial.print(" cm");
    if (odometry.readSqual(&squal) == PAA5160_OK) {
      Serial.print("  | SQUAL = ");
      Serial.print(squal);
      Serial.print(" (features = ");
      Serial.print((uint16_t)squal * 4);
      Serial.print(")");
    }
    Serial.println();
  } else {
    Serial.println("read displacement failed.");
  }
  delay(500);
}
