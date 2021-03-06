#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_MODE_INPUT			0x0000
#define GPIO_MODE_OUTPUT		0x0001
#define GPIO_MODE_ALT			0x0002
#define GPIO_MODE_ANALOG		0x0004

#define GPIO_CONF_PULL_UP		0x0008
#define GPIO_CONF_PULL_DOWN		0x0010
#define GPIO_CONF_OPENDRAIN		0x0020

#define GPIO_INT_FALLING		0x0100
#define GPIO_INT_RISING			0x0200

int gpio_init(unsigned int index, unsigned int flags);
void gpio_close(unsigned int index);
void gpio_put(unsigned int index, int v);
unsigned int gpio_get(unsigned int index);

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/gpio.h>
#include INCPATH

#endif /* __GPIO_H__ */
