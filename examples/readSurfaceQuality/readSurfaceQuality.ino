/*!
 * @file readSurfaceQuality.ino
 * @brief 演示 readSqual：输出表面特征质量与有效特征点数，用于判断表面对光流跟踪是否可用。
 * @details SQUAL 越大表示当前帧的有效特征越多；有效特征点数量 = 值 × 4。
 *          在不同表面上各测一次（例如亚光纸、金属、木纹），可以直观对比可跟踪性。
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

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("surface quality demo: keep the module at 10~27 mm above the tracking surface");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (I2C).");
    delay(1000);
  }
  /* 用固定挡位，保证不同表面之间可比。 */
  if (odometry.setCpi(20) != PAA5160_OK) {
    Serial.println("setCpi failed.");
  }
}

void loop()
{
  uint8_t squal = 0;
  float totalX = 0.0f;
  float totalY = 0.0f;

  if (odometry.readSqual(&squal) == PAA5160_OK) {
    Serial.print("SQUAL = ");
    Serial.print(squal);
    Serial.print("  features = ");
    Serial.print((uint16_t)squal * 4);
    if (squal < 20) {
      Serial.print("  (weak surface: displacement may be unreliable)");
    }
    Serial.println();
  } else {
    Serial.println("readSqual failed.");
  }

  if (odometry.getTotalDisplacement(&totalX, &totalY) == PAA5160_OK) {
    Serial.print("X = ");
    Serial.print(totalX, 3);
    Serial.print(" cm, Y = ");
    Serial.print(totalY, 3);
    Serial.println(" cm");
  }
  delay(1000);
}
