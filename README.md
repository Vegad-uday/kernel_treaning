# kernel_treaning
This repo includes Linux kernel modules: a basic character device driver and an I2C driver for the MPU6500 sensor. The char device handles open, read, write, and release operations, while the MPU6500 driver reads gyroscope data via I2C and exposes it through a device file for user-space access.
