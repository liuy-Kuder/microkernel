#include "microkernel.h"
#include "mk_gpio_drv.h"
#include "stdio.h"
#include "stm32f4xx_ll_gpio.h"

uint32_t mk_gp_write(struct gpio_drv_t * gp, int pin)
{
	LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9|LL_GPIO_PIN_10);
    return 0;
}

uint32_t mk_gp_read(struct gpio_drv_t * gp, int pin)
{
	LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_9|LL_GPIO_PIN_10);
    return 0;
}

int16_t mk_gp_write_reg(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpio_drv_t * gpio = (struct gpio_drv_t *)kobj->priv;
	LL_GPIO_ResetOutputPin(gpio->GPIOx, gpio->pin|LL_GPIO_PIN_10);
    return 0;
}

int16_t mk_gp_read_reg(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpio_drv_t * gpio = (struct gpio_drv_t *)kobj->priv;
	LL_GPIO_SetOutputPin(gpio->GPIOx, gpio->pin|LL_GPIO_PIN_10);
    return 0;
}

static int16_t gpio_reg(struct kobj_t * kobj, uint16_t cmd, void * buf)
{
 //   struct gpio_drv_t * gpio = (struct gpio_drv_t *)kobj->priv;
	struct gpio_driver_t * gpio = (struct gpio_driver_t *)kobj->priv;
	if(cmd)
	  LL_GPIO_SetOutputPin(gpio->GPIOx, gpio->pin);
	else
	  LL_GPIO_ResetOutputPin(gpio->GPIOx, gpio->pin);
	return 0;
}

struct device_t * mk_gp_probe_bak(struct driver_t * drv)
{
	struct gpio_drv_t * gp[2] = NULL;
	struct device_t * dev;
	for(uint8_t i = 0; i < 2; i++)
	{
		gp[i] = malloc(sizeof(struct gpio_drv_t));
		if(!gp[i])
		 {
			return NULL;
		 }

		gp[i]->GPIOx = GPIOF;
		if(i == 1)
		  gp[i]->pin = LL_GPIO_PIN_10;
		else
		  gp[i]->pin = LL_GPIO_PIN_9;
		kobj_add_regular(drv->kobj, i == 0 ? "gpio9" : "gpio10", NULL, NULL, gpio_reg, gp[i]);
	}
#if 0
	gp = malloc(sizeof(struct gpio_drv_t));
	if(!gp)
	 {
		return NULL;
	 }

	gp->GPIOx = GPIOF;
	gp->pin = LL_GPIO_PIN_9;
	kobj_add_regular(drv->kobj, "gpio9", NULL, NULL, gpio_reg, gp);
	kobj_add_regular(drv->kobj, "gpio10", NULL, NULL, gpio_reg, gp);
#endif
	return dev;
}

struct device_t * mk_gp_probe(struct driver_t * drv,struct driver_info_t * drv_info)
{
	struct gpio_driver_t * gpio_drv = (struct gpio_driver_t *)drv_info->drv_private;

	//struct gpio_driver_t * gpio = NULL;
	struct device_t * dev;
	uint16_t i = 0;
	while(i < drv_info->drv_num)
	{
	/*	gpio = malloc(sizeof(struct gpio_driver_t));
		if(!gpio)
		 {
			return NULL;
		 }
		gpio->node_name = gpio_drv->node_name;
		gpio->GPIOx = gpio_drv->GPIOx;
		gpio->pin = gpio_drv->pin;
*/
		kobj_add_regular(drv->kobj, gpio_drv->node_name, NULL, NULL, gpio_reg,gpio_drv);
		i++;
		gpio_drv++;
	}
	return NULL;
}

static void mk_gp_remove(struct device_t * dev)
{
	struct gpio_drv_t * gp = (struct gpio_drv_t *)dev->priv;

	if(gp)
	{
		unregister_adc(gp);
		//free_device_name(gp->name);
		free(gp);
	}
}

static void mk_gp_suspend(struct device_t * dev){}

static void mk_gp_resume(struct device_t * dev){}

static struct driver_t gpio = {
	.name		= "gpio",
	.probe		= mk_gp_probe,
	.remove		= mk_gp_remove,
	.suspend	= mk_gp_suspend,
	.resume		= mk_gp_resume,
};

void mk_gpio_driver_init(void)
{
	register_driver(&gpio);
}

void mk_gpio_driver_exit(void)
{
	unregister_driver(&gpio);
}

//探索总线驱动
void Probe_BusDriver(void)
{
	struct driver_t * drv;
	drv = search_driver("gpio");
//if(drv != NULL)
	// drv->probe(drv);
}

struct gpio_driver_t gpio_drv_info[] = {
	{
		.node_name = "gpio9",
		.GPIOx = GPIOF,
		.pin = LL_GPIO_PIN_9,
	},
	{
		.node_name = "gpio10",
		.GPIOx = GPIOF,
		.pin = LL_GPIO_PIN_10,
	},
	{
		.node_name = "gpio11",
		.GPIOx = GPIOF,
		.pin = LL_GPIO_PIN_11,
	}
};


/*
设备描述
{
	.driver_name = "gpio"//驱动名字
		{
			.node_name = "gpio9",//驱动节点名称
			.group = GPIOF,//参数
			.pin = LL_GPIO_PIN_9,
		},
		{
			.node_name = "gpio10",//驱动节点名称
			.group = GPIOF,//参数
			.pin = LL_GPIO_PIN_10,
		}
}
*/
void init_driver(void)
{
	struct driver_info_t drv = {
		.drv_name = "gpio",
		.drv_num = ARRAY_SIZE(gpio_drv_info),
		.drv_private = &gpio_drv_info,
	};

	struct driver_t * drv_t;
	drv_t = search_driver(drv.drv_name);
	if(drv_t != NULL)
	 drv_t->probe(drv_t,&drv);
}
