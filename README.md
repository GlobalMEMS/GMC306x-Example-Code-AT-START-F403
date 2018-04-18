AT-START-F403 + GMC306x example code
====================================

Requirements
-----------
- AT-START-F403 development board
- GMC306x: GMC306 or GMC306A magnetometer

I2C Connections
---------------
- Use I2C1
  - SCL: PB8
  - SDA: PB9
- GMC306x I2C 7-bit slave address: 0x0C

Configuration for GMC306A
=========================
Default sensor is GMC306. For GMC306A, uncomment the `USE_GMC306A` macro in the **gmc306x.h** for correct configuration.
```
/* Uncomment below macro for GMC306A */
//#define USE_GMC306A
```
