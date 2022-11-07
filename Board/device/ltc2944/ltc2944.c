#include "microkernel.h"
#include "ltc2944.h"
#include <stdarg.h>
#include "i2cSim.h"

/*
offset: 寄存器位置偏移
buf: 缓冲
size: 数量
*/
static size_t ltc2944_read(struct kobj_t * kobj, size_t offset, void * buf, size_t size)
{
	struct device_priv_t * ltc2944 = (struct device_priv_t *)kobj->priv;
	struct i2c_t * i2c = (struct i2c_t *)ltc2944->bus->priv;
	struct i2c_msg_t read_msgs[2];
	int ret = 0;
	unsigned char b[1] = {0};
	unsigned char readBuf[2];
	b[0] = LTC2943_STATUS_REG + offset;//寄存器地址
	read_msgs[0].addr = ltc2944->addr;//寄存器地址
	read_msgs[0].flags = 0;//写指令
	read_msgs[0].buf = b;
	read_msgs[0].len = 1;//长度

	read_msgs[1].addr = ltc2944->addr;//寄存器地址
	read_msgs[1].flags = I2C_M_RD;//读指令
	read_msgs[1].buf = readBuf;
	read_msgs[1].len = size;//长度
	ret = i2c->xfer(i2c,read_msgs,2);
	switch(b[0])
	 {
		case LTC2943_ACCUM_CHARGE_REG:
		//	*(float*)buf = 100*(float)(accumulate_code * (0.34E-3) * 4096 * (50E-3))/(0.025*4096);
			break;
		case LTC2943_CHARGE_THRESH_H_REG:
			break;
		case LTC2943_CHARGE_THRESH_L_REG:
			break;
		case LTC2943_VOLTAGE_REG:
			uint16_t voltCode = readBuf[0]<<8|readBuf[1];
			*(float*)buf = (float)(67.8 * voltCode / 0xffff);
			break;
		case LTC2943_VOLTAGE_THRESH_H_REG:
			break;
		case LTC2943_VOLTAGE_THRESH_L_REG:
			break;
		case LTC2943_CURRENT_REG:
			uint16_t current_code = readBuf[0]<<8|readBuf[1];
			*(float*)buf = (64.0/50.0) * (current_code - 0x7fff)* 1000.0 / 0x7fff;
			break;
		case LTC2943_CURRENT_THRESH_H_REG:
			break;
		case LTC2943_CURRENT_THRESH_L_REG:
			break;
		case LTC2943_TEMPERATURE_REG:
			uint16_t temptrue_code = readBuf[0]<<8|readBuf[1];
			*(float*)buf = temptrue_code*(510.0/65535.0) - 273.15;
			break;
		case LTC2943_TEMPERATURE_THRESH_H_REG:
			break;
		case LTC2943_TEMPERATURE_THRESH_L_REG:
			break;
		default:
			break;
	 }
	return ret;
}

/*
offset: 寄存器位置偏移
buf: 缓冲
size: 数量
*/
static size_t ltc2944_write(struct kobj_t * kobj, size_t offset, void * buf, size_t size)
{
	struct device_priv_t * ltc2944 = (struct device_priv_t *)kobj->priv;
	struct i2c_t * i2c = (struct i2c_t *)ltc2944->bus->priv;
	struct i2c_msg_t msgs;

	msgs.addr = ltc2944->addr;//寄存器地址
	msgs.flags = 0;//写指令
	msgs.buf = buf;
	msgs.len = size;//长度
	return i2c->xfer(i2c,&msgs,1);
}

static int16_t ltc2944_ioctl(struct kobj_t * kobj, uint16_t cmd, void * buf)
{
	struct device_priv_t * ltc2944 = (struct device_priv_t *)kobj->priv;
	switch(cmd)
	{
		case LTC2944_GET_ALCC_PIN_STATE:
			return ltc2944->read_alcc_pin(ltc2944->alcc_group,ltc2944->alcc_pin);
		default:return -1;
	}
}

int ltc2944_init(struct kobj_t * i2c_driver)
{
	uint8_t param[2] = {LTC2943_CONTROL_REG, ADC_MODE | SETS_COULOMB_PRESCA_FACTOR_M | ALCC_CONFIG_MODE | SHUTDOWN_MODE};
	return ltc2944_write(i2c_driver,LTC2943_CONTROL_REG,param,2);
}

//注册LTC2944设备
struct device_t * register_ltc2944_dev(struct register_info_t * reg_info)
{
	struct device_ltc2944_info_t * i2c_dev = (struct device_ltc2944_info_t *)reg_info->device_private;
	struct device_t * dev = NULL;
	struct device_priv_t * dev_priv = NULL;
	uint16_t i = 0;
	int ret = 0;
	while(i < reg_info->device_num)
	 {
		dev = MK_MALLOC(sizeof(struct device_t));
		if(!dev)
		  return NULL;

		dev_priv = MK_MALLOC(sizeof(struct device_priv_t));
		if(!dev_priv)
		  return NULL;

		dev->name = i2c_dev->device_name;
		dev->type = DEVICE_TYPE_I2C;
		dev_priv->addr = i2c_dev->device_addr;
		dev_priv->alcc_group = i2c_dev->alcc_group;
		dev_priv->alcc_pin = i2c_dev->alcc_pin;
		dev_priv->read_alcc_pin = i2c_dev->read_alcc_pin;
		//--------------------查找通信总线进行捆绑----------------------
		dev->driver = search_driver(reg_info->driver_name);
		if(dev->driver != NULL)
		  dev_priv->bus = kobj_search(dev->driver->kobj, i2c_dev->node_name);//寻找节点
		//------------------------------------------------------------
		dev->kobj = kobj_alloc_regular(dev->name, ltc2944_read, ltc2944_write, ltc2944_ioctl, LTC2943_TEMPERATURE_THRESH_L_REG, dev_priv);
		ret = ltc2944_init(dev->kobj);
		if(ret < 0)//结束本次循环、不注册
		 {
			i++;
			i2c_dev++;
			kobj_remove_self(dev->kobj);
			MK_FREE(dev);
			continue;
		 }
		if(!register_device(dev))
		 {
			kobj_remove_self(dev->kobj);
			MK_FREE(dev);
			return NULL;
		 }
		i++;
		i2c_dev++;
	 }
	return dev;
}

//卸载驱动
int unregister_ltc2944_dev(void)
{
	struct device_t * dev;  
	int ret = 0;
	dev = search_device("ltc2944", DEVICE_TYPE_I2C);
	if(dev && unregister_device(dev))
	 {
		ret = kobj_remove_self(dev->kobj);
		MK_FREE(dev);
	 }
	return ret;
}

struct device_ltc2944_info_t i2c_dev_info[] = {
	{
		.node_name = "i2cSim0",
		.device_name = "ltc2944",
		.device_addr = 0x64,
		.alcc_group = GPIOB,
		.alcc_pin = LL_GPIO_PIN_1,
		.read_alcc_pin = HAL_GPIO_ReadPin,
	}
};

struct device_t * init_ltc2944_device(void)
{

	struct register_info_t dev_t = {
		.driver_name = "i2cSim",
		.device_num = ARRAY_SIZE(i2c_dev_info),
		.device_private = &i2c_dev_info,
	};
	return register_ltc2944_dev(&dev_t);
}
