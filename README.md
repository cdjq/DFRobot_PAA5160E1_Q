# DFRobot_PAA5160E1_Q

Arduino driver library for the Fermion: PAA5160-E1 optical odometry module (SKU: SEN0777). The module integrates the PixArt PAA5160E1-Q optical tracking chip and an on-board MCU; the host reads the accumulated X/Y displacement in centimeters over I2C or UART.

* Working distance 10~27 mm, speed up to 2.5 m/s, resolution up to 20000 cpi
* Host interface: I2C (default address 0x11) or MODBUS-RTU UART (9600 8N1, default slave address 0x32)
* The interface is selected by the module `mode` pin at power-on and cannot be switched at runtime

## Connection

The module exposes a Fermion connector. Only one interface is active, decided at power-on by the `mode` pin (external pull-down = I2C, pull-up = UART).

| Host | I2C mode | UART mode |
| --- | --- | --- |
| UNO | SDA=A4, SCL=A5, VCC=3.3V/5V, GND | no `Serial1`, use I2C |
| Mega2560 | SDA=20, SCL=21, VCC=3.3V/5V, GND | Serial1: TX1=18 → module RX, RX1=19 ← module TX |
| ESP32 | SDA=GPIO21, SCL=GPIO22, VCC=3.3V, GND | `Serial1.begin(9600, SERIAL_8N1, rxPin, txPin)` |

> The module I/O is 3.3 V. When using a 5 V host (UNO / Mega2560), check the bus pull-up level and add a level shifter if needed.

## Installation

1. Download this repository and copy the `DFRobot_PAA5160E1_Q` folder into your Arduino `libraries` folder
2. Or install the ZIP through the Arduino IDE "Add .ZIP Library"
3. Restart the IDE and open the examples from `DFRobot_PAA5160E1_Q`

## Examples

* `readDisplacement` — **main example** (I2C): initializes, prints the chip product id / revision and SQUAL, and periodically prints the X/Y total displacement in cm; setup also covers one `setCpi` and one `setOrientation`
* `readDisplacementUART` — UART mode: the same displacement readout over MODBUS RTU
* `setResolution` — shows `setCpi` switching CPI steps and how the increments change
* `calibrateScale` — computes and writes the X/Y calibration scale from a known distance, then reads it back
* `readSurfaceQuality` — shows `readSqual` feature points to judge whether a surface is trackable
* `setOrientation` — shows the swap/invert combinations and their effect on X/Y

## Methods

```cpp
/**
 * @fn DFRobot_PAA5160E1_Q_I2C
 * @brief Constructor, binds the I2C bus and the module slave address
 * @param pWire I2C bus object
 * @param i2cAddr module slave address, default 0x11 (module addr pin pulled down)
 * @n The address is 0x12 when the addr pin is pulled up; it is sampled at power-on and cannot be changed at runtime.
 * @return None
 */
DFRobot_PAA5160E1_Q_I2C(TwoWire *pWire = &Wire, uint8_t i2cAddr = 0x11);

/**
 * @fn DFRobot_PAA5160E1_Q_UART
 * @brief UART constructor, binds the hardware serial object
 * @param serial UART object (HardwareSerial), default &Serial1
 * @param baudRate fixed 9600, kept only for compatibility
 * @n Fixed frame: 8 data bits, no parity, 1 stop bit, no hardware flow control (9600 8N1).
 * @return None
 */
DFRobot_PAA5160E1_Q_UART(HardwareSerial *serial = &Serial1, uint32_t baudRate = 9600);

/**
 * @fn begin
 * @brief Initializes the bus and reads the module identity to confirm communication
 * @return bool true = module found; false = no device or communication failure
 * @n Only valid for the interface the module booted with; I2C and UART are mutually exclusive.
 */
bool begin(void);

/**
 * @fn getPAA_ID
 * @brief Reads the PAA5160E1-Q chip product id
 * @param productId 2-byte product id
 * @return bool true = read succeeded
 * @n Read from the module identity register (I2C 0x10 / UART input 0x0006); returns 0 when the chip is not detected.
 */
bool getPAA_ID(uint16_t &productId);

/**
 * @fn getPAA_RevisionID
 * @brief Reads the PAA5160E1-Q chip firmware revision
 * @param firmwareVer 3-byte revision
 * @return bool true = read succeeded
 * @n Read from the module identity register (I2C 0x12, 3 bytes / UART input 0x0007~0x0008).
 */
bool getPAA_RevisionID(uint8_t firmwareVer[3]);

/**
 * @fn getTotalDisplacement
 * @brief Gets the software-side accumulated X/Y total displacement
 * @param totalX_cm X total displacement in cm
 * @param totalY_cm Y total displacement in cm
 * @return int PAA5160_OK = success; PAA5160_ERR_IO = communication failure
 * @n 1. The value keeps accumulating while the module stays powered; it is lost after power-off;
 * @n 2. This call does not clear the accumulator; the module reads delta counts, converts by CPI and applies the calibration scale.
 */
int getTotalDisplacement(float *totalX_cm, float *totalY_cm);

/**
 * @fn resetTotalDisplacement
 * @brief Clears the software-side accumulated total displacement
 * @return int PAA5160_OK = success; PAA5160_ERR_IO = communication failure
 * @n Clears only the displacement accumulator, the calibration scale is not affected.
 */
int resetTotalDisplacement(void);

/**
 * @fn setCalibrationScale
 * @brief Sets the X/Y displacement calibration scale, stored in flash
 * @param scaleX X calibration scale
 * @param scaleY Y calibration scale
 * @return int PAA5160_OK = saved; PAA5160_ERR_PARAM = out of range; PAA5160_ERR_IO = communication failure
 * @n 1. The scale means "actual distance / module reading": 100 cm actual with a 109 cm reading gives 100.0/109.0;
 * @n 2. Suggested range 0.5 ~ 2.0; out of range means the calibration distance is too short and this call returns a parameter error;
 * @n 3. The value is stored in the module flash and reloaded automatically after power-on.
 */
int setCalibrationScale(float scaleX, float scaleY);

/**
 * @fn setCpi
 * @brief Sets the optical flow CPI resolution, stored in flash
 * @param cpi CPI step, 1~200 (200 steps)
 * @return int PAA5160_OK = success; PAA5160_ERR_PARAM = invalid argument; PAA5160_ERR_IO = communication failure
 * @n 1. A new CPI only affects later increments, already accumulated displacement is not recalculated;
 * @n 2. High CPI: better fine-motion resolution, more noise and easier to overflow, needs a higher read rate;
 * @n 3. Low CPI: less noise and higher speed support, but very small motion is lost.
 */
int setCpi(uint16_t cpi);

/**
 * @fn readSqual
 * @brief Reads SQUAL surface feature quality to judge tracking confidence
 * @param squal raw register value 0~255
 * @return int PAA5160_OK = success; PAA5160_ERR_PARAM = null pointer; PAA5160_ERR_IO = communication failure
 * @n Feature points = squal × 4; a low value means the surface has too few features and displacement is less reliable.
 */
int readSqual(uint8_t *squal);

/**
 * @fn setOrientation
 * @brief Sets the optical flow axes (swap XY, invert X/Y), not stored in flash
 * @param swapXY 1 = swap X and Y; 0 = keep
 * @param invertX 1 = invert X; 0 = keep
 * @param invertY 1 = invert Y; 0 = keep
 * @return bool true = configured
 * @n 1. Allowed values: 0 = do nothing, 1 = enable that transform;
 * @n 2. When both swap and invert are enabled the module swaps first and then inverts;
 * @n 3. The setting is not stored in flash and returns to default after power-on.
 */
bool setOrientation(bool swapXY, bool invertX, bool invertY);
```

## Compatibility

* Boards: Arduino UNO, Mega2560, ESP32 and others (`architectures=*`)
* Dependencies: **`DFRobot_RTU`** (UART / MODBUS-RTU transport, `depends=DFRobot_RTU`; install https://github.com/DFRobot/DFRobot_RTU first) and `Wire` for I2C mode
* Module I2C address: 0x11 (addr pin low, default) or 0x12 (high)
* UART: MODBUS RTU, 9600 8N1, default slave address 0x32; CRC16, request framing, response parsing and timeout all come from DFRobot_RTU

## History

- 2026/10/14 - V1.0.0  First release: I2C / UART interfaces and 6 examples

## Credits

Written by wxzed(xiao.wu@dfrobot.com), 2026. (Welcome to our [website](https://www.dfrobot.com/))
