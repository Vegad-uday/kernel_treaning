#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include<linux/proc_fs.h>


#define DEVICE_NAME "gyroscope_mpu"
#define CLASS_NAME  "mpu6500"

// MPU register
#define MPU6500_ADD 0x68
#define WHO_AM_I    0x75

#define GYRO_X_H    0x43 
#define GYRO_X_L    0x44
#define GYRO_Y_H    0x45
#define GYRO_Y_L    0x46
#define GYRO_Z_H    0x47
#define GYRO_Z_L    0x48

static dev_t dev_id;
static struct class *mpu_class;
static struct cdev mpu_cdev;
static struct proc_dir_entry *parent;
static struct i2c_client *mpu_client;
static int gyro[3];

static struct work_struct mpu6500_work;
//static void mpu6500_work_func(struct work_struct *work);

static void mpu6500_work_func(struct work_struct *work){
    pr_info("mpu6500: Work Function running...\n");

    gyro[0] = (i2c_smbus_read_word_data(mpu_client, GYRO_X_H) | i2c_smbus_read_word_data(mpu_client, GYRO_X_L)); 
    gyro[1] = (i2c_smbus_read_word_data(mpu_client, GYRO_Y_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Y_L)); 
    gyro[2] = (i2c_smbus_read_word_data(mpu_client, GYRO_Z_H) | i2c_smbus_read_word_data(mpu_client, GYRO_Z_L)); 

    pr_info("gyroscope x axis: %d\n", gyro[0]);    
    pr_info("gyroscope y axis: %d\n", gyro[1]);    
    pr_info("gyroscope z axis: %d\n", gyro[2]);    
}

// file opration for proc/mpu6500_gyro file
static int open_proc(struct inode *inode, struct file *file){
    pr_info("proc file opend.....\t");
    return 0;
}
static int release_proc(struct inode *inode, struct file *file){
    pr_info("proc file released.....\n");
    return 0;
}
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset){
    pr_info("proc file read...\n");
    schedule_work(&mpu6500_work);
    return 1;
}
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off){
    pr_info("proc file write...\n");
    
    return 1;
}

// file opretion function for /dev/mpu6500
static int file_open(struct inode *inode, struct file *file){
    pr_info("mpu6500: device file opened\n");
    return 0;
}

static int file_release(struct inode *inode, struct file *file){
    pr_info("mpu6500: device file closed\n");
    return 0;
}

static ssize_t file_read(struct file *file, char *out_buf, size_t len, loff_t *offset){
    schedule_work(&mpu6500_work);
    return 0;
}

static ssize_t file_write(struct file *file, const char *buf, size_t len, loff_t *offset){
    pr_info("mpu6500: data writing ...\n");
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = file_open,
    .release = file_release,
    .read    = file_read,
    .write   = file_write,
};
// proc file oprations struct
static struct proc_ops proc_fops = {
        .proc_open = open_proc,
        .proc_read = read_proc,
        .proc_write = write_proc,
        .proc_release = release_proc
};
static const struct of_device_id mpu_of_match[] = {
    { .compatible = "uday,mpu6500" },
    { }
};
MODULE_DEVICE_TABLE(of, mpu_of_match);

static int mpu_probe(struct i2c_client *client){
    int ret;
    mpu_client = client;

    if (alloc_chrdev_region(&dev_id, 0, 1, DEVICE_NAME) < 0) {
        pr_err("mpu6500: failed to allocate char device\n");
        return -1;
    }

    pr_info("MAJOR NUMBER: %d, MINOR NUMBER: %d\n", MAJOR(dev_id), MINOR(dev_id));
    pr_info("mpu6500: module loading...\n");

    cdev_init(&mpu_cdev, &fops);
    ret = cdev_add(&mpu_cdev, dev_id, 1);
    if (ret) {
        unregister_chrdev_region(dev_id, 1);
        pr_err("mpu6500: failed to add cdev\n");
        return ret;
    }

    mpu_class = class_create( CLASS_NAME);
    if (IS_ERR(mpu_class)) {
        cdev_del(&mpu_cdev);
        unregister_chrdev_region(dev_id, 1);
        pr_err("mpu6500: failed to create class\n");
        return PTR_ERR(mpu_class);
    }

    if (device_create(mpu_class, NULL, dev_id, NULL, DEVICE_NAME) == NULL) {
        class_destroy(mpu_class);
        cdev_del(&mpu_cdev);
        unregister_chrdev_region(dev_id, 1);
        pr_err("mpu6500: failed to create device\n");
        return -1;
    }
    parent = proc_mkdir("mpu6500_gyro",NULL);
        
    if( parent == NULL ){
        pr_info("Error creating proc entry");
    	class_destroy(mpu_class);      
    }
        
    proc_create("mpu6500_proc", 0666, parent, &proc_fops);

    INIT_WORK(&mpu6500_work, mpu6500_work_func);
    return 0;
}

static void mpu_remove(struct i2c_client *client){
    flush_work(&mpu6500_work);
    device_destroy(mpu_class, dev_id);
    class_destroy(mpu_class);
    cdev_del(&mpu_cdev);
    unregister_chrdev_region(dev_id, 1);

    pr_info("mpu6500: driver removed\n");
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
    .probe    = mpu_probe, 
    .remove   = mpu_remove,
    .id_table = mpu_id,
};

module_i2c_driver(mpu6500_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Uday");
MODULE_DESCRIPTION("MPU6500 I2C Driver ");

