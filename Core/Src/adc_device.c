#include "microkernel.h"
#include "adc_driver.h"
#include <stdio.h>

static int16_t adc_read_vreference(struct kobj_t * kobj, void * buf, size_t size)
{
	//struct adc_t * adc = (struct adc_t *)kobj->priv;
	//return sprintf(buf, "%Ld.%06LdV", adc->vreference / (uint64_t)(1000 * 1000), adc->vreference % (uint64_t)(1000 * 1000));
    return 0;
}

static int16_t adc_read_resolution(struct kobj_t * kobj, void * buf, size_t size)
{
	//struct adc_t * adc = (struct adc_t *)kobj->priv;
	//return sprintf(buf, "%d", adc->resolution);
    return 0;
}

static int16_t adc_read_nchannel(struct kobj_t * kobj, void * buf, size_t size)
{
	//struct adc_t * adc = (struct adc_t *)kobj->priv;
	return 0;
}

static int16_t adc_ioctl(struct kobj_t * kobj, uint16_t cmd, ...)
{
	return 0;
}

static int16_t adc_read_raw_channel(struct kobj_t * kobj, void * buf, size_t size)
{
	//struct adc_t * adc = (struct adc_t *)kobj->priv;
	//int channel = strtoul(kobj->name + strlen("raw"), NULL, 0);
	//return sprintf(buf, "%d", adc_read_raw(adc, channel));
    return 0;
}

static int16_t adc_read_voltage_channel(struct kobj_t * kobj, void * buf, size_t size)
{
	//struct adc_t * adc = (struct adc_t *)kobj->priv;
	//int channel = strtoul(kobj->name + strlen("voltage"), NULL, 0);
	//int voltage = adc_read_voltage(adc, channel);
	//return sprintf(buf, "%Ld.%06LdV", voltage / (uint64_t)(1000 * 1000), voltage % (uint64_t)(1000 * 1000));
    return 0;
}

struct adc_t * search_adc(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_ADC);
	if(!dev)
		return NULL;
	return (struct adc_t *)dev->priv;
}

struct device_t * register_adc(struct adc_t * adc, struct driver_t * drv)
{
	struct device_t * dev;
	char buf[64];
	int i;

	if(!adc || !adc->name || (adc->resolution <= 0) || (adc->nchannel <= 0) || !adc->read)
		return NULL;

	dev = malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = strdup(adc->name);
	dev->type = DEVICE_TYPE_ADC;
	dev->driver = drv;
	dev->priv = adc;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "vreference", adc_read_vreference, NULL, NULL, adc);
	kobj_add_regular(dev->kobj, "resolution", adc_read_resolution, NULL, NULL, adc);
	kobj_add_regular(dev->kobj, "nchannel", adc_read_nchannel, NULL,NULL, adc);
	kobj_add_regular(dev->kobj, "ioctl", adc_read_nchannel, NULL,adc_ioctl, adc);
	
	for(i = 0; i< adc->nchannel; i++)
	{
		sprintf(buf, "raw%d", i);
		kobj_add_regular(dev->kobj, buf, adc_read_raw_channel, NULL,NULL, adc);
	}
	for(i = 0; i< adc->nchannel; i++)
	{
		sprintf(buf, "voltage%d", i);
		kobj_add_regular(dev->kobj, buf, adc_read_voltage_channel, NULL,NULL, adc);
	}

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		free(dev->name);
		free(dev);
		return NULL;
	}
	return dev;
}

void unregister_adc(struct adc_t * adc)
{
	struct device_t * dev;

	if(adc && adc->name)
	{
		dev = search_device(adc->name, DEVICE_TYPE_ADC);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			free(dev->name);
			free(dev);
		}
	}
}

uint32_t adc_read_raw(struct adc_t * adc, int channel)
{
	if(adc && adc->read)
	{
		if(channel < 0)
			channel = 0;
		else if(channel > adc->nchannel - 1)
			channel = adc->nchannel - 1;
		return adc->read(adc, channel);//读取寄存器值
	}
	return 0;
}

int adc_read_voltage(struct adc_t * adc, int channel)
{
	if(adc && adc->read)
	{
		if(channel < 0)
			channel = 0;
		else if(channel > adc->nchannel - 1)
			channel = adc->nchannel - 1;
		return ((int64_t)adc->read(adc, channel) * adc->vreference) / ((1 << adc->resolution) - 1);//读取寄存器值
	}
	return 0;
}
