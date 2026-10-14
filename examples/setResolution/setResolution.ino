/*!
 * @file setResolution.ino
 * @brief 演示 setCpi 切换 CPI 挡位，并观察同一段位移下读数增量的变化。
 * @details 交替设置低挡位与高挡位，各清零一次累加位移后再读数，便于对比分辨率差异。
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

static void showDisplacement(const char *tag)
{
  float totalX = 0.0f;
  float totalY = 0.0f;

  if (odometry.getTotalDisplacement(&totalX, &totalY) == PAA5160_OK) {
    Serial.print(tag);
    Serial.print("  X = ");
    Serial.print(totalX, 4);
    Serial.print(" cm, Y = ");
    Serial.print(totalY, 4);
    Serial.println(" cm");
  } else {
    Serial.print(tag);
    Serial.println("  read failed");
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("setCpi demo: low CPI loses small motion, high CPI is finer but noisier");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (I2C).");
    delay(1000);
  }

  if (odometry.setCpi(5) != PAA5160_OK) {
    Serial.println("setCpi(5) failed.");
  }
  odometry.resetTotalDisplacement();
  delay(500);
  showDisplacement("[CPI  5 =  500 cpi]");

  if (odometry.setCpi(200) != PAA5160_OK) {
    Serial.println("setCpi(200) failed.");
  }
  odometry.resetTotalDisplacement();
  delay(500);
  showDisplacement("[CPI 200 = 20000 cpi]");

  Serial.println("Now move the module and watch the printed increments change.");
}

void loop()
{
  showDisplacement("[current]");
  delay(1000);
}
