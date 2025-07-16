#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>         
#include<linux/uaccess.h>   

#define mem_size	1024 

static dev_t dev_id;
static struct class *dev_class;
static struct cdev st_cdev;
uint8_t *kernel_buffer;

static int      open(struct inode *inode, struct file *file);
static int      release(struct inode *inode, struct file *file);
static ssize_t  read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  write(struct file *filp, const char *buf, size_t len, loff_t * off);

struct file_operations fops ={
    .owner= THIS_MODULE,
    .read= read,
    .write= write,
    .open= open,
    .release= release
};

static int open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

static int release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
static ssize_t write (struct file *filp, const char __user *buf, size_t len, loff_t *off){
	
	if( copy_from_user(kernel_buffer, buf, len) ){
            pr_err("Data Write : Err!\n");
        }
        pr_info("Data Write : Done!\n");
        return len;
}

static ssize_t read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        if( copy_to_user(buf, kernel_buffer, mem_size) ){
            pr_err("Data Read : Err!\n");
        }
        pr_info("Data Read : Done!\n");
        return mem_size;
}

static int hello_start(void){
    
    int test = alloc_chrdev_region(&dev_id , 0, 1, "Uday");
    if(test != 0 ){
        printk(KERN_ERR "ERROR IN ALLOCATE MAJOR NUMBER FOR DEVICE");
        return -1;
    }
    printk(KERN_INFO "MAJOR NUMBER:%d & MINOR NUMBER:%d\n",MAJOR(dev_id),MINOR(dev_id));
    printk(KERN_INFO "MODULE LOADING... \n");
    
    cdev_init(&st_cdev,&fops);
    cdev_add(&st_cdev,dev_id, 1);

    dev_class= class_create("uday");
    device_create(dev_class,NULL,dev_id,NULL,"uday_device");

    if((kernel_buffer = kmalloc(mem_size , GFP_KERNEL)) == 0){
        pr_info("Cannot allocate memory in kernel\n");
    }
        
    return 0;
}

static void hello_end(void){
    printk(KERN_INFO "MODULE EXTING... \n");
    device_destroy(dev_class,dev_id);
    class_destroy(dev_class);
    cdev_del(&st_cdev);
    unregister_chrdev_region(dev_id, 1);
    printk(KERN_INFO "DEVICE FILE DESTROYED\n");
    printk(KERN_INFO "MODULE EXTI DONE\n");
}

module_init(hello_start);
module_exit(hello_end);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Uday");
MODULE_DESCRIPTION("Sample charecter driver");
