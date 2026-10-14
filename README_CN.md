# DFRobot_PAA5160E1_Q

Fermion: PAA5160-E1 光学里程计（SKU: SEN0777）的 Arduino 驱动库。模块内部集成 PixArt PAA5160E1-Q 光学追踪芯片与一颗板载 MCU；主机通过 I2C 或 UART 读取累计的 X/Y 位移（单位 cm）。

* 工作距离 10~27 mm，最高 2.5 m/s，最高 20000 cpi
* 对外 I2C（默认地址 0x11）或 MODBUS RTU UART（9600 8N1，默认从站地址 0x32）
* 接口模式由模块 `mode` 引脚在上电时决定，运行中不可切换

## 接线

模块的对外接口是 Fermion 接口，两种模式二选一（**上电时**由 mode 脚决定：外部下拉 = I2C，上拉 = UART）。

| 主机 | I2C 模式 | UART 模式 |
| --- | --- | --- |
| UNO | SDA=A4、SCL=A5、VCC=3.3V/5V、GND | 无 `Serial1`，请用 I2C |
| Mega2560 | SDA=20、SCL=21、VCC=3.3V/5V、GND | Serial1：TX1=18 → 模块 RX，RX1=19 ← 模块 TX |
| ESP32 | SDA=GPIO21、SCL=GPIO22、VCC=3.3V、GND | `Serial1.begin(9600, SERIAL_8N1, rxPin, txPin)` 指定引脚 |

> 模块 I/O 为 3.3 V 电平。用 5 V 主机（UNO / Mega2560）时请确认总线上拉电平，必要时加电平转换。

## 库安装

1. 下载本仓库，把 `DFRobot_PAA5160E1_Q` 目录放入 Arduino 的 `libraries` 目录
2. 或通过 Arduino IDE 的「加载 .ZIP 库」安装压缩包
3. 重启 Arduino IDE，在示例中找到 `DFRobot_PAA5160E1_Q`

## 示例

* `readDisplacement` —— **主示例**（I2C）：初始化、打印芯片产品号/版本与 SQUAL，周期输出 X/Y 总位移（cm）；setup 内覆盖一次 `setCpi` 与 `setOrientation`
* `readDisplacementUART` —— UART 模式：同样的位移读取与打印
* `setResolution` —— 演示 `setCpi` 切换 CPI 挡位并观察位移增量变化
* `calibrateScale` —— 按已知实际距离计算并写入 X/Y 校准系数，再读取验证
* `readSurfaceQuality` —— 演示 `readSqual` 输出有效特征点数，判断表面可用性
* `setOrientation` —— 演示 `setOrientation` 的交换/翻转组合及其对 X/Y 输出的影响

## 方法

```cpp
/**
 * @fn DFRobot_PAA5160E1_Q_I2C
 * @brief 构造函数，绑定 I2C 总线与模块从机地址
 * @param pWire I2C 总线对象
 * @param i2cAddr 模块从机地址，默认 0x11（模块 addr 脚外部下拉）
 * @n 模块 addr 脚上拉时地址为 0x12；地址在上电时采样，运行中不可改。
 * @return 无
 */
DFRobot_PAA5160E1_Q_I2C(TwoWire *pWire = &Wire, uint8_t i2cAddr = 0x11);

/**
 * @fn DFRobot_PAA5160E1_Q_UART
 * @brief UART 接口构造函数，绑定硬件串口对象
 * @param serial UART 串口对象（HardwareSerial），默认 &Serial1
 * @param baudRate 串口波特率，固定为 9600，仅做兼容传参
 * @n 串口固定参数：8 数据位、无校验、1 停止位、无硬件流控（9600 8N1）。
 * @return 无
 */
DFRobot_PAA5160E1_Q_UART(HardwareSerial *serial = &Serial1, uint32_t baudRate = 9600);

/**
 * @fn begin
 * @brief 通讯初始化，读取模块身份寄存器确认通讯正常
 * @return bool true=通讯正常识别到传感器；false=无设备/通讯失败
 * @n 本接口只适用于模块上电时为对应模式的情况；I2C 与 UART 从上电起互斥。
 */
bool begin(void);

/**
 * @fn getPAA_ID
 * @brief 读取 PAA5160E1-Q 芯片产品号
 * @param productId 输出 2 字节产品号
 * @return bool true=读取成功
 * @n 该值来自模块对外身份寄存器（I2C 0x10 / UART 输入寄存器 0x0006），模块识别不到芯片时读回 0。
 */
bool getPAA_ID(uint16_t &productId);

/**
 * @fn getPAA_RevisionID
 * @brief 读取 PAA5160E1-Q 芯片固件版本
 * @param firmwareVer 输出 3 字节版本号
 * @return bool true=读取成功
 * @n 该值来自模块对外身份寄存器（I2C 0x12 3 字节 / UART 输入寄存器 0x0007~0x0008）。
 */
bool getPAA_RevisionID(uint8_t firmwareVer[3]);

/**
 * @fn getTotalDisplacement
 * @brief 获取软件侧持续累加的 X/Y 总位移
 * @param totalX_cm 输出 X 轴总位移，单位 cm
 * @param totalY_cm 输出 Y 轴总位移，单位 cm
 * @return int PAA5160_OK=获取成功；PAA5160_ERR_IO=通讯失败
 * @n 1. 传感器不断电时数值持续累加，断电后累加值丢失；
 * @n 2. 本接口不会清零内部累加器；模块内部自动读取 Δcount、按 CPI 换算并应用校准系数。
 */
int getTotalDisplacement(float *totalX_cm, float *totalY_cm);

/**
 * @fn resetTotalDisplacement
 * @brief 清零模块软件侧累加的总位移，把 totalX、totalY 归 0
 * @return int PAA5160_OK=清零成功；PAA5160_ERR_IO=通讯失败
 * @n 只清模块的 XY 位移累加，校准系数不受影响。
 */
int resetTotalDisplacement(void);

/**
 * @fn setCalibrationScale
 * @brief 设置 X/Y 位移校准比例系数，掉电保存
 * @param scaleX X 轴校准比例
 * @param scaleY Y 轴校准比例
 * @return int PAA5160_OK=保存成功；PAA5160_ERR_PARAM=系数超出范围；PAA5160_ERR_IO=通讯失败
 * @n 1. 系数含义为「实际距离 / 模块读数」，例如实际移动 100 cm、模块输出 109 cm，则系数取 100.0/109.0；
 * @n 2. 建议范围 0.5 ~ 2.0，超出说明校准距离不足，本接口返回参数错误；
 * @n 3. 写入后存入模块 Flash，模块断电重上电自动加载生效。
 */
int setCalibrationScale(float scaleX, float scaleY);

/**
 * @fn setCpi
 * @brief 设置光流传感器 CPI 分辨率，掉电保存
 * @param cpi CPI 挡位，取值 1~200，共 200 档
 * @return int PAA5160_OK=设置成功；PAA5160_ERR_PARAM=参数非法；PAA5160_ERR_IO=通讯失败
 * @n 1. 修改 CPI 只对后续新读取的位移增量生效，已经累加的总位移不会重算；
 * @n 2. 高 CPI：微小位移分辨好，噪声更大、更易溢出，需要更高读取速率；
 * @n 3. 低 CPI：噪声小、支持更高运动速度，但微小位移会丢失。
 */
int setCpi(uint16_t cpi);

/**
 * @fn readSqual
 * @brief 读取 SQUAL 表面特征质量，判断光流跟踪可信度
 * @param squal 输出原始寄存器值 0~255
 * @return int PAA5160_OK=读取成功；PAA5160_ERR_PARAM=空指针；PAA5160_ERR_IO=通讯失败
 * @n 有效特征点数量 = squal × 4；值偏低表示表面特征不足，位移可信度下降。
 */
int readSqual(uint8_t *squal);

/**
 * @fn setOrientation
 * @brief 设置光流坐标系方向（交换 XY、翻转 X/Y 方向），不掉电保存
 * @param swapXY 1=交换 X、Y 轴；0=不交换
 * @param invertX 1=翻转 X 轴方向；0=不翻转
 * @param invertY 1=翻转 Y 轴方向；0=不翻转
 * @return bool true=配置成功
 * @n 1. 各参数可选值：0 = 不处理，1 = 启用对应变换；
 * @n 2. 同时开启交换与翻转时，模块先执行 XY 交换、再执行方向翻转；
 * @n 3. 配置不掉电保存，重新上电后回到默认方向。
 */
bool setOrientation(bool swapXY, bool invertX, bool invertY);
```

## 兼容性

* 开发板：Arduino UNO、Mega2560、ESP32 等（`architectures=*`）
* 依赖库：**`DFRobot_RTU`**（UART / MODBUS RTU 传输层，`depends=DFRobot_RTU`，请先安装 https://github.com/DFRobot/DFRobot_RTU ）；`Wire`（I2C 模式）
* 模块 I2C 地址：0x11（addr 脚下拉，默认）或 0x12（上拉）
* UART：MODBUS RTU，9600 8N1，默认从站地址 0x32；CRC16、请求组帧、响应解析与超时均由 DFRobot_RTU 提供

## 历史

- 2026/10/14 - V1.0.0  首次发布：I2C / UART 两种接口与 6 个示例

## 致谢

* 作者：wxzed（xiao.wu@dfrobot.com）
* 芯片：PixArt PAA5160E1-Q
