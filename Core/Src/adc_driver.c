#include "microkernel.h"
#include "adc_driver.h"

//底层寄存器操作读取ADC值
static uint32_t adc_d1_read(struct adc_t * adc, int channel)
{
	uint32_t val = 1000;
    return val;
}

struct device_t * adc_d1_probe(struct driver_t * drv,struct driver_info_t * drv_info)
{
	struct adc_t * adc = NULL;
	struct device_t * dev;

	adc = malloc(sizeof(struct adc_t));
	if(!adc)
	{
		return NULL;
	}

	adc->name = "adc_test";
	adc->vreference = 251479;
	adc->resolution = 12;
	adc->nchannel = 2;
	adc->read = adc_d1_read;

	if(!(dev = register_adc(adc, drv)))
	{
		free_device_name(adc->name);
		free(adc);
		return NULL;
	}
	return dev;
}

static void adc_d1_remove(struct device_t * dev)
{
	struct adc_t * adc = (struct adc_t *)dev->priv;

	if(adc)
	{
		unregister_adc(adc);
		free_device_name(adc->name);
		free(adc);
	}
}

static void adc_d1_suspend(struct device_t * dev)
{

}

static void adc_d1_resume(struct device_t * dev)
{

}

static struct driver_t adc_d1 = {
	.name		= "adc-d1",
	.probe		= adc_d1_probe,
	.remove		= adc_d1_remove,
	.suspend	= adc_d1_suspend,
	.resume		= adc_d1_resume,
};

void adc_d1_driver_init(void)
{
	register_driver(&adc_d1);
}

void adc_d1_driver_exit(void)
{
	unregister_driver(&adc_d1);
}
