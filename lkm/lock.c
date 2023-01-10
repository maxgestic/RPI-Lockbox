/*
 ============================================================================
 Name        : lock.c
 Author      : Maximilian Ring
 Version     : 0.0.1
 Copyright   : See Abertay copyright notice
 Description : RPi Zero Lock Driver
 ============================================================================
 */
#include "lock.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/seq_file.h>

static int DevBusy = 0;
static int MajorNum = 100;
static struct class*  ClassName  = NULL;
static struct device* DeviceName = NULL;

gpio_pin apin;

static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "lock: device_open(%p)\n", file);

	if (DevBusy)
		return -EBUSY;

	DevBusy++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "lock: device_release(%p)\n", file);
	DevBusy--;

	module_put(THIS_MODULE);
	return 0;
}

static long int device_ioctl(struct file *file, unsigned int cmd, long unsigned int arg){

	printk("lock: Device IOCTL invoked : 0x%x - %u\n" , cmd , cmd);

	switch (cmd) {
	case LOCK_TOGGLE:
		memset(&apin, 0, sizeof(apin));
		gpio_request(17, "Details");
		apin.value = gpio_get_value(17);
		printk("lock: Pin Value: %i Setting to %i\n", apin.value, !apin.value);
		gpio_direction_output(17, 0);
		gpio_set_value(17, !apin.value);
		break;
	default:
			printk("lock: command format error\n");
	}

	return 0;
}

struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

static int __init lock_init(void){
	int ret_val;
	ret_val = 0;

	   printk(KERN_INFO "lock: Initializing the lock dev\n");
	   MajorNum = register_chrdev(0, DEVICE_NAME, &Fops);
	      if (MajorNum<0){
	         printk(KERN_ALERT "lock: failed to register a major number\n");
	         return MajorNum;
	      }
	   printk(KERN_INFO "lock: registered with major number %d\n", MajorNum);

	   ClassName = class_create(THIS_MODULE, CLASS_NAME);
	   if (IS_ERR(ClassName)){
	      unregister_chrdev(MajorNum, DEVICE_NAME);
	      printk(KERN_ALERT "lock: Failed to register device class\n");
	      return PTR_ERR(ClassName);
	   }
	   printk(KERN_INFO "lock: device class registered\n");

	   DeviceName = device_create(ClassName, NULL, MKDEV(MajorNum, 0), NULL, DEVICE_NAME);
	   if (IS_ERR(DeviceName)){
	      class_destroy(ClassName);
	      unregister_chrdev(MajorNum, DEVICE_NAME);
	      printk(KERN_ALERT "lock: Failed to create the device\n");
	      return PTR_ERR(DeviceName);
	   }
	   printk(KERN_INFO "lock: device class created\n");

	return 0;
}

static void __exit lock_exit(void){
	   device_destroy(ClassName, MKDEV(MajorNum, 0));
	   class_unregister(ClassName);
	   class_destroy(ClassName);
	   unregister_chrdev(MajorNum, DEVICE_NAME);
	   gpio_free(apin.pin);
	   printk(KERN_INFO "lock: Module removed\n");
}
module_init(lock_init);
module_exit(lock_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maximilian Ring");
MODULE_DESCRIPTION("RPi Zero Lock Driver");
MODULE_VERSION("0.0.1");
