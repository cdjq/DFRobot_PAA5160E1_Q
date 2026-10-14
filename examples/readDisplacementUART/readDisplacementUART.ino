/*!
 * @file readDisplacementUART.ino
 * @brief UART 模式示例：以 MODBUS RTU 9600 8N1 读取身份、表面质量与 X/Y 总位移。
 * @details setup 里完成完整配置链：begin() → 打印芯片产品号 → setCpi(20) → resetTotalDisplacement()；
 *          loop 里周期打印 SQUAL 与 X/Y 总位移（cm）。
 * @n 接线（UART 模式，模块 mode 脚需要外部上拉）：
 * @n Mega2560: 模块 SCL/RX ← Mega TX1(18)，模块 SDA/TX → Mega RX1(19)，VCC=3.3V 或 5V，GND=GND
 * @n ESP32   : 模块 SCL/RX ← ESP32 TX，模块 SDA/TX → ESP32 RX，VCC=3.3V，GND=GND（示例内用 Serial1 并指定引脚）
 * @n UNO     : 无 Serial1，请改用 readDisplacement（I2C 模式）示例
 * @n 模块 I/O 为 3.3 V 电平；用 5 V 主机做 UART 时请确认电平或加转换。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */
#include "DFRobot_PAA5160E1_Q_UART.h"

/* ESP32 请在 setup 里用 Serial1.begin(9600, SERIAL_8N1, rxPin, txPin) 指定实际引脚。 */
DFRobot_PAA5160E1_Q_UART odometry(&Serial1, 9600);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Fermion PAA5160-E1 optical odometry (UART, MODBUS RTU)");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (UART).");
    delay(1000);
  }
  Serial.print("MODBUS slave address: 0x");
  Serial.println(odometry.getSlaveAddress(), HEX);

  uint16_t productId = 0;
  if (odometry.getPAA_ID(productId)) {
    Serial.print("PAA5160E1-Q product id: 0x");
    Serial.println(productId, HEX);
  }

  if (odometry.setCpi(20) != PAA5160_OK) {
    Serial.println("setCpi failed.");
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
