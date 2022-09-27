#ifndef __ADC_DRIVER_H_
#define __ADC_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "microkernel.h"

struct adc_t
{
	char * name;//名称
	int vreference;//参考点
	int resolution;//分辨率
	int nchannel;//通道

	uint32_t (*read)(struct adc_t * adc, int channel);//读取参数
	void * priv;//私有变量
};

struct adc_t * search_adc(const char * name);
struct device_t * register_adc(struct adc_t * adc, struct driver_t * drv);
void unregister_adc(struct adc_t * adc);

uint32_t adc_read_raw(struct adc_t * adc, int channel);
int adc_read_voltage(struct adc_t * adc, int channel);
void adc_d1_driver_init(void);
void adc_d1_driver_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */
