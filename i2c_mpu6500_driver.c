#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#define DEVICE_NAME "mpu6500"
#define CLASS_NAME  "uday"

//MPU register
#define MPU6500_ADD 0x68
#define WHO_AM_I 0x75

#define GYRO_X_H	0X43 
#define GYRO_X_L	0X44
#define GYRO_Y_H	0X45
#define GYRO_Y_L	0X46
#define GYRO_Z_H	0x47
#define GYRO_Z_L	0X48

static dev_t dev_id;
static struct class *mpu_class;
static struct cdev mpu_cdev;
static struct i2c_client *mpu_client;
static int gyro[3];

static int file_open(struct inode *inode, struct file *file){
    pr_info("mpu6500: device file opened\n");
    return 0;
}

static int file_release(struct inode *inode, struct file *file){
    pr_info("mpu6500: device file closed\n");
    return 0;
}

static ssize_t file_read(struct file *file, char *out_buf, size_t len, loff_t *offset){
 
    pr_info("data reading ...\n");    
    int reg = i2c_smbus_read_word_data(mpu_client, WHO_AM_I);
    pr_info("device id is:%x \n",reg);
    gyro[0] =(i2c_smbus_read_word_data(mpu_client, GYRO_X_H) | i2c_smbus_read_word_data(mpu_client, GYRO_X_L)); 
    gyro[1] =(i2c_smbus_read_word_data(mpu_client, GYRO_Y_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Y_L)); 
    gyro[2] =(i2c_smbus_read_word_data(mpu_client, GYRO_Z_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Z_L)); 


    pr_info("gyroscope x axis:%d\n",gyro[0]/131);    
    pr_info("gyroscope y axis:%d\n",gyro[1]/131);    
    pr_info("gyroscope z axis:%d\n",gyro[2]/131);    
    
    return 0;
}

static ssize_t file_write(struct file *file, const char *buf, size_t len, loff_t *offset){
    pr_info("data Writeing ...\n");
    int ret = 0;
      return ret;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = file_open,
    .release = file_release,
    .read    = file_read,
    .write   = file_write,
};

static const struct of_device_id mpu_of_match[] = {
    { .compatible = "uday,mpu6500" },
    { }
};
MODULE_DEVICE_TABLE(of, mpu_of_match);

static int mpu_probe(struct i2c_client *client){
    int ret;
    mpu_client = client;

    if( alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME)< 0){
        pr_err("mpu6500: not allocate char device\n");
        return -1;
    }
    printk(KERN_INFO "MAJOR NUMBER:%d & MINOR NUMBER:%d\n",MAJOR(dev_id),MINOR(dev_id));
    printk(KERN_INFO "MODULE LOADING... \n");


    cdev_init(&mpu_cdev, &fops);
    ret = cdev_add(&mpu_cdev, dev_id, 1);
    if (ret) {
        unregister_chrdev_region(dev_id, 1);
        pr_err("mpu6500: not added to cdev\n");
        return ret;
    }

    mpu_class = class_create(CLASS_NAME);

    if (device_create(mpu_class, NULL, dev_id, NULL, DEVICE_NAME) == NULL) {
        class_destroy(mpu_class);
        cdev_del(&mpu_cdev);
        unregister_chrdev_region(dev_id, 1);
        pr_err("mpu6500: not create  device\n");
        return -1;
    }

/*    int reg = i2c_smbus_read_word_data(mpu_client, WHO_AM_I);
    pr_info("device id is:%x \n",reg);
    int regx =(i2c_smbus_read_word_data(mpu_client, GYRO_X_H) | i2c_smbus_read_word_data(mpu_client, GYRO_X_L) ); 
    int regy =(i2c_smbus_read_word_data(mpu_client, GYRO_Y_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Y_L) ); 
    int regz =(i2c_smbus_read_word_data(mpu_client, GYRO_Z_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Z_L) ); 

    pr_info("gryo axis x :%d \n",regx);
    pr_info("gryo axis y :%d \n",regy);
    pr_info("gryo axis z :%d \n",regz);
*/
    return 0;
}

static void mpu_remove(struct i2c_client *client){
    device_destroy(mpu_class, dev_id);
    class_destroy(mpu_class);
    cdev_del(&mpu_cdev);
    unregister_chrdev_region(dev_id, 1);

    pr_info("mpu6500: Driver remove\n");
}

static const struct i2c_device_id mpu_id[] = {
    { "mpu6500", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, mpu_id);

static struct i2c_driver mpu6500_driver = {
    .driver = {
        .name           = "mpu6500_driver",
        .of_match_table = of_match_ptr(mpu_of_match),
    },
    .probe = mpu_probe, 
    .remove    = mpu_remove,
    .id_table  = mpu_id,
};

module_i2c_driver(mpu6500_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("uday");
MODULE_DESCRIPTION("I2C Driver for MPU6500");
