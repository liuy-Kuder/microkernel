#ifndef __MK_GPIO_DRV_H_
#define __MK_GPIO_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "microkernel.h"
#include "stm32f4xx_ll_gpio.h"
struct gpio_drv_t
{
/*	char * name;//名称
	uint32_t (*write)(struct gpio_drv_t * gp, int pin);//写入参数
	uint32_t (*read)(struct gpio_drv_t * gp, int pin);//写入参数
	struct kobj_t * kobj;
	void * priv;//私有变量
*/
	GPIO_TypeDef *GPIOx;
	uint32_t pin;
};

struct gpio_driver_t{
	char* node_name;
	uint32_t pin;
	GPIO_TypeDef *GPIOx;
};

struct gpio_drv_t * search_mkgp(const char * name);
struct device_t * register_mkgp(struct gpio_drv_t * gp, struct driver_t * drv);
void unregister_mkgp(struct gpio_drv_t * gp);

void mk_gpio_driver_init(void);
void mk_gpio_driver_exit(void);
void Probe_BusDriver(void);
void init_driver(void);
#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */
