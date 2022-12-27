/*
 ============================================================================
 Name        : lockbox.c
 Author      : Max Ring
 Version     : 0.0.1
 Copyright   : GNU GPL v3.0
 Description : Lockbox Driver
 ============================================================================
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/seq_file.h>

#define DEVICE_NAME "lkboxdev"
#define DEVICE_CLASS "lkboxcls"

static int DevBusy = 0;
static int MajorNum = 100;
static struct class*  ClassName  = NULL;
static struct device* DeviceName = NULL;

static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "lockbox: device_open(%p)\n", file);

	if (DevBusy)
		return -EBUSY;

	DevBusy++;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "lockbox: device_release(%p)\n", file);
	DevBusy--;

	module_put(THIS_MODULE);
	return 0;
}

static int device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	printk("lockbox: Device IOCTL invoked : 0x%x - %u\n" , cmd , cmd);

	switch (cmd) {
	case 0x65:
		printk("lockbox: TEST\n");
		break;
	default:
			printk("lockbox: command format error\n");
	}

	return 0;
}

struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

static int __init lockbox_init(void){
	int ret_val;
	ret_val = 0;

	   printk(KERN_INFO "lockbox: Initializing the piio\n");
	   MajorNum = register_chrdev(0, DEVICE_NAME, &Fops);
	      if (MajorNum<0){
	         printk(KERN_ALERT "piio: failed to register a major number\n");
	         return MajorNum;
	      }
	   printk(KERN_INFO "lockbox: registered with major number %d\n", MajorNum);

	   ClassName = class_create(THIS_MODULE, CLASS_NAME);
	   if (IS_ERR(ClassName)){
	      unregister_chrdev(MajorNum, DEVICE_NAME);
	      printk(KERN_ALERT "lockbox: Failed to register device class\n");
	      return PTR_ERR(ClassName);
	   }
	   printk(KERN_INFO "lockbox: device class registered\n");

	   DeviceName = device_create(ClassName, NULL, MKDEV(MajorNum, 0), NULL, DEVICE_NAME);
	   if (IS_ERR(DeviceName)){
	      class_destroy(ClassName);
	      unregister_chrdev(MajorNum, DEVICE_NAME);
	      printk(KERN_ALERT "lockbox: Failed to create the device\n");
	      return PTR_ERR(DeviceName);
	   }
	   printk(KERN_INFO "lockbox: device class created\n");

	return 0;
}

static void __exit lockbox_exit(void){
	   device_destroy(ClassName, MKDEV(MajorNum, 0));
	   class_unregister(ClassName);
	   class_destroy(ClassName);
	   unregister_chrdev(MajorNum, DEVICE_NAME);
	   printk(KERN_INFO "lockbox: Module removed\n");
}
module_init(lockbox_init);
module_exit(lockbox_exit);
MODULE_LICENSE("GNU GPL v3");
MODULE_AUTHOR("Max Ring");
MODULE_DESCRIPTION("Lockbox Driver");
MODULE_VERSION("0.0.1");
