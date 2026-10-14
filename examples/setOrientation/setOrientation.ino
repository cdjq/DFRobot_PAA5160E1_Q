/*!
 * @file setOrientation.ino
 * @brief 演示 setOrientation 的交换/翻转组合及其对 X/Y 输出的影响。
 * @details 依次设置三种方向：默认、交换 XY、翻转 X 与 Y；每种设置下清零位移再移动同一方向，
 *          观察 X/Y 读数分别落到哪个轴上、正负号如何变化。
 * @n 同时开启交换与翻转时，模块先执行 XY 交换、再执行方向翻转；配置不掉电保存。
 * @n 接线（I2C 模式）：UNO SDA=A4/SCL=A5；Mega2560 SDA=20/SCL=21；ESP32 SDA=GPIO21/SCL=GPIO22。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */
#include "DFRobot_PAA5160E1_Q_I2C.h"

DFRobot_PAA5160E1_Q_I2C odometry(&Wire, 0x11);

static void applyAndReport(const char *tag, bool swapXY, bool invertX, bool invertY)
{
  float totalX = 0.0f;
  float totalY = 0.0f;

  if (!odometry.setOrientation(swapXY, invertX, invertY)) {
    Serial.print(tag);
    Serial.println(": setOrientation failed");
    return;
  }
  odometry.resetTotalDisplacement();
  delay(2000);
  if (odometry.getTotalDisplacement(&totalX, &totalY) == PAA5160_OK) {
    Serial.print(tag);
    Serial.print(": X = ");
    Serial.print(totalX, 4);
    Serial.print(" cm, Y = ");
    Serial.print(totalY, 4);
    Serial.println(" cm");
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("orientation demo: move the module in the same direction for each step");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (I2C).");
    delay(1000);
  }
  odometry.setCpi(20);

  applyAndReport("default (no swap, no invert)", false, false, false);
  applyAndReport("swap X and Y            ", true, false, false);
  applyAndReport("invert X and Y          ", false, true, true);

  /* 恢复默认方向，方便后续使用。 */
  odometry.setOrientation(false, false, false);
}

void loop()
{
  float totalX = 0.0f;
  float totalY = 0.0f;

  if (odometry.getTotalDisplacement(&totalX, &totalY) == PAA5160_OK) {
    Serial.print("X = ");
    Serial.print(totalX, 4);
    Serial.print(" cm, Y = ");
    Serial.print(totalY, 4);
    Serial.println(" cm");
  }
  delay(1000);
}
