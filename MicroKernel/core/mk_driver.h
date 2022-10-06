#ifndef __MK_DRIVER_H_
#define __MK_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mk_list.h"

struct device_t;

struct driver_info_t{
	char* drv_name;//设备名称
	uint8_t drv_num;//设备数量
	void * drv_private;//私有变量
};

struct driver_t
{
	struct kobj_t * kobj;
	struct hlist_node node;

	char * name;
	struct device_t * (*probe)(struct driver_t * drv,struct driver_info_t * drv_info);
	void (*remove)(struct device_t * dev);
	void (*suspend)(struct device_t * dev);
	void (*resume)(struct device_t * dev);
};

struct driver_t * search_driver(const char * name);
uint8_t register_driver(struct driver_t * drv);
uint8_t unregister_driver(struct driver_t * drv);
void driver_pure_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRIVER_H__ */
