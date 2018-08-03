AT-START-F403 + GMC306x example code
====================================

Requirements
-----------
- AT-START-F403 development board
- Sensor Fusion Arduino Daughter Board V1.0: GMC306A is on the sensor board

I2C Connections
---------------
- Use I2C1
  - SCL: PB8
  - SDA: PB9
- GMC306x I2C 7-bit slave address: 0x0C

Configuration for GMC306
========================
Default sensor is GMC306A. For GMC306, comment out the `USE_GMC306A` macro in the **gmc306x.h** for correct configuration.
```
/* Uncomment below macro for GMC306A */
#define USE_GMC306A
```
