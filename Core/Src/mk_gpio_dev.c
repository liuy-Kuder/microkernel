#include "microkernel.h"
#include <stdio.h>

static int16_t led_ioctl(struct kobj_t * kobj, uint16_t cmd, void * buf)
{
    struct kobj_t * gp = (struct kobj_t *)kobj->priv;
	if(gp->ioctl != NULL)
    {
        gp->ioctl(gp,cmd,NULL);
    }
	return 0;
}

struct device_info_t{
	uint8_t * device_name;
	uint8_t * node_name;
	uint8_t * attribute;
};

struct register_info_t{
	uint8_t * driver_name;
	uint8_t device_num;
	void * device_private;
};

//注册LED设备
struct device_t * register_led_dev(struct register_info_t * dev_info)
{
	struct device_info_t * gpio_dev = (struct device_info_t *)dev_info->device_private;
	struct device_t * dev;
	uint16_t i = 0;

	while(i < dev_info->device_num)
	 {
		dev = malloc(sizeof(struct device_t));
		if(!dev)
			return NULL;
		dev->name = gpio_dev->device_name;
		dev->type = DEVICE_TYPE_LED;//类型是LED
		//--------------------查找通信总线-----------------------------
		dev->driver = search_driver(dev_info->driver_name);
		struct kobj_t * gp = kobj_search(dev->driver->kobj, gpio_dev->node_name);//寻找节点
		//------------------------------------------------------------
		dev->kobj = kobj_alloc_directory(dev->name);
		kobj_add_regular(dev->kobj, gpio_dev->attribute , NULL, NULL, led_ioctl, gp);//赋能LED开关属性

		if(!register_device(dev))
		{
			kobj_remove_self(dev->kobj);
			//free(dev->name);
			free(dev);
			return NULL;
		}
		i++;
		gpio_dev++;
	 }
	return dev;
#if 0
	struct device_t * dev;
	dev = malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

    dev->name = strdup("red");//红灯
	dev->type = DEVICE_TYPE_LED;//类型是LED
    //--------------------查找通信总线-----------------------------
    dev->driver = search_driver("gpio");
    struct kobj_t * gp = kobj_search(dev->driver->kobj, "gpio9");//寻找节点
    //------------------------------------------------------------
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "switch", NULL, NULL, led_ioctl, gp);//赋能LED开关属性

	if(!register_device(dev))
	 {
		kobj_remove_self(dev->kobj);
		free(dev->name);
		free(dev);
		return NULL;
	 }
	return dev;
#endif
}

//卸载驱动
int unregister_led_dev(void)
{
	struct device_t * dev;  
	int ret = 0;
	dev = search_device("red", DEVICE_TYPE_LED);
	if(dev && unregister_device(dev))
	 {
		ret = kobj_remove_self(dev->kobj);
		free(dev->name);
		free(dev);
	 }
	return ret;
}


struct device_info_t dev_info_t[] = {
		{
			.device_name = "red",
			.node_name = "gpio9",
			.attribute = "switch",
		},
		{
			.device_name = "green",
			.node_name = "gpio10",
			.attribute = "switch",
		},
};

/*
设备描述
{
	.driver_name = "gpio"//哪一个驱动
	{
		.device_name = "red",//创建设备名称
		.node = "gpio9",//调用的节点
		.attribute = "switch",//属性
	},
	{
		.device_name = "green",
		.node = "gpio10",
		.attribute = "switch",
	}
}
*/

void init_led_device(void)
{
	struct register_info_t dev_t = {
		.driver_name = "gpio",
		.device_num = ARRAY_SIZE(dev_info_t),
		.device_private = &dev_info_t,
	};
	register_led_dev(&dev_t);
}
