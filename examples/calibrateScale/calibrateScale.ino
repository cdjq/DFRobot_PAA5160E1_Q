/*!
 * @file calibrateScale.ino
 * @brief 演示按已知实际距离计算并写入 X/Y 校准系数，然后读回验证。
 * @details 步骤：清零累加位移 → 沿 X 轴移动一段已知距离 D_actual → 读出模块读数 D_read
 *          → 计算 scale = D_actual / D_read → setCalibrationScale(scale, scale) → 再移动同一段距离验证。
 * @n 系数范围建议 0.5~2.0；超出说明校准距离太短，请增加移动距离后重试。
 * @n 接线（I2C 模式）：UNO SDA=A4/SCL=A5；Mega2560 SDA=20/SCL=21；ESP32 SDA=GPIO21/SCL=GPIO22。
 * @copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license     The MIT License (MIT)
 * @author      wxzed(xiao.wu@dfrobot.com)
 * @version     V1.0.0
 * @date        2026-10-14
 * @url         https://github.com/DFRobot/DFRobot_PAA5160E1_Q
 */
#include "DFRobot_PAA5160E1_Q_I2C.h"

/* 实测使用的已知移动距离，单位 cm。请按你的滑台/直尺输入真实值。 */
#define CALIBRATION_DISTANCE_CM 20.0f

DFRobot_PAA5160E1_Q_I2C odometry(&Wire, 0x11);

static bool measure(float *x_cm, float *y_cm)
{
  return odometry.getTotalDisplacement(x_cm, y_cm) == PAA5160_OK;
}

void setup()
{
  float x = 0.0f;
  float y = 0.0f;
  float scale = 1.0f;

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("calibration demo: move the module a known distance, then press reset to redo");

  while (!odometry.begin()) {
    Serial.println("Sensor begin failed, check wiring and the mode pin (I2C).");
    delay(1000);
  }
  odometry.setCpi(20);

  Serial.println("Step 1/3 resetting the accumulator. Move the module now...");
  odometry.resetTotalDisplacement();
  delay(3000);

  if (!measure(&x, &y)) {
    Serial.println("read displacement failed; abort.");
    return;
  }
  Serial.print("Step 2/3 measured X = ");
  Serial.print(x, 4);
  Serial.print(" cm (actual ");
  Serial.print(CALIBRATION_DISTANCE_CM, 2);
  Serial.println(" cm)");

  if (x > 0.1f) {
    scale = CALIBRATION_DISTANCE_CM / x;
  }
  Serial.print("computed scale = ");
  Serial.println(scale, 5);

  if (odometry.setCalibrationScale(scale, scale) == PAA5160_OK) {
    Serial.println("Step 3/3 scale written to the module (stored in flash).");
  } else {
    Serial.println("setCalibrationScale failed: distance too short, use a longer move.");
  }

  odometry.resetTotalDisplacement();
  delay(3000);
  if (measure(&x, &y)) {
    Serial.print("after calibration X = ");
    Serial.print(x, 4);
    Serial.println(" cm  (should be close to the actual distance)");
  }
}

void loop()
{
  float x = 0.0f;
  float y = 0.0f;

  if (measure(&x, &y)) {
    Serial.print("X = ");
    Serial.print(x, 4);
    Serial.print(" cm, Y = ");
    Serial.print(y, 4);
    Serial.println(" cm");
  }
  delay(1000);
}
