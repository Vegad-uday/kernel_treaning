# kernel_treaning
This repo includes Linux kernel modules: a basic character device driver and an I2C driver for the MPU6500 sensor. The char device handles open, read, write, and release operations, while the MPU6500 driver reads gyroscope data via I2C and exposes it through a device file for user-space access.

# MPU6500 Gyroscope I2C Linux Kernel Driver

## Description

This project provides a Linux kernel module to interface with the **MPU6500** gyroscope over the **I2C bus**. The module includes:

1. An I2C-based character device driver.
2. A procfs interface (`/proc/mpu6500_gyro`) for gyroscope data.
3. A standalone character device (`/dev/uday_device`) demonstrating memory operations.

## Features

- Communicates with the MPU6500 via I2C.
- Provides gyroscope data for X, Y, Z axes.
- Includes procfs support to trigger data acquisition via user-space.
- Offers simple character device functionality for learning purposes.

---

## Files

| File               | Description                                                   |
|--------------------|---------------------------------------------------------------|
| `mpu6500_driver.c` | Main I2C driver for MPU6500 with `/dev/mpu6500` interface      |
| `proc_mpu6500.c`   | Adds `/proc/mpu6500_gyro` to read gyro data using workqueue    |
| `uday_char.c`      | A sample character driver with basic read/write functionality |
| `Makefile`         | Kernel module build file                                      |
| `Kconfig`          | Kernel configuration file to integrate with kernel menuconfig |

---

## Building the Modules

```bash
make
```
Loading the Modules
```
sudo insmod mpu6500_driver.ko
sudo insmod proc_mpu6500.ko
sudo insmod uday_char.ko
```
Unloading the Modules
```
sudo rmmod mpu6500_driver
sudo rmmod proc_mpu6500
sudo rmmod uday_char
```
Using the Driver

    Device file: /dev/mpu6500

    Proc file: /proc/mpu6500_gyro

    Character device: /dev/uday_device

# Trigger data read from MPU6500 via procfs
```
cat /proc/mpu6500_gyro
```
# Read from MPU6500 device
```
cat /dev/mpu6500
```
# Write to uday device
```
echo "Test data" > /dev/uday_device
```
Ensure your device tree includes a compatible node:
```
&i2c1 {
    mpu6500@68 {
        compatible = "uday,mpu6500";
        reg = <0x68>;
    };
};
```

---

## ✅ Makefile

```makefile
obj-m += mpu6500_driver.o
obj-m += proc_mpu6500.o
obj-m += uday_char.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```
 Kconfig

If you want to integrate these drivers into a kernel tree for selection with make menuconfig, here's a sample Kconfig:

menu "MPU6500 Support"

config MPU6500_DRIVER
	tristate "MPU6500 I2C Character Driver"
	depends on I2C
	help
	  This enables the I2C driver for MPU6500 with /dev interface.

config PROC_MPU6500
	tristate "MPU6500 Proc Interface Driver"
	help
	  This adds a /proc interface to read MPU6500 gyroscope data.

config UDAY_CHAR
	tristate "Simple Character Device Driver"
	help
	  A basic character driver for testing read/write from user-space.

endmenu

You can then include this in your kernel's drivers/Makefile like:
```
obj-$(CONFIG_MPU6500_DRIVER) += mpu6500_driver.o
obj-$(CONFIG_PROC_MPU6500)   += proc_mpu6500.o
obj-$(CONFIG_UDAY_CHAR)      += uday_char.o
```
