#ifndef LOCK_H
#define LOCK_H

#include <linux/ioctl.h>

typedef struct gpio_pin {
	char desc[16];
	unsigned int pin;
	int value;
	char opt;
} gpio_pin;

#define LOCK_TOGGLE 0x65

#define  DEVICE_NAME "lockdev"
#define  CLASS_NAME  "lockdev"

#endif
