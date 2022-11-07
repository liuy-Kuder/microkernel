#include "microkernel.h"
#include "stm32f4xx_HAL_gpio.h"
#include "i2c.h"

//发送Byte
static unsigned char i2c_outb(struct i2c_gpio_pdata_t * pdat, unsigned char c)
{

}
//读取Byte
static unsigned char i2c_inb(struct i2c_gpio_pdata_t * pdat)
{

}

//发送字符串
static int sendbytes(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msg)
{
	const unsigned char * temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	return wrcount;
}

static int i2c_algo_bit_xfer(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msgs, int num)
{
	struct i2c_msg_t * pmsg;
	int i, ret;
	unsigned short nak_ok;

	for(i = 0; i < num; i++)
	 {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;
		if(!(pmsg->flags & I2C_M_NOSTART))
		 {
			if(i)
			 {
				i2c_repstart(pdat);
			 }
			ret = bit_do_address(pdat, pmsg);
			if((ret != 0) && !nak_ok)
			 {
				goto bailout;
			 }
		 }
		if(pmsg->flags & I2C_M_RD)
		 {
			ret = readbytes(pdat, pmsg);
			if(ret < pmsg->len)
			 {
				if (ret >= 0)
					ret = -1;
				goto bailout;
			 }
		 }
		else
		 {
			ret = sendbytes(pdat, pmsg);
			if(ret < pmsg->len)
			 {
				if (ret >= 0)
					ret = -1;
				goto bailout;
			 }
		 }
	 }
	ret = i;

bailout:
	i2c_stop(pdat);
	return ret;
}

static int readbytes(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msg)
{
	int inval;
	int rdcount = 0;
	unsigned char *temp = msg->buf;
	int count = msg->len;
	const unsigned flags = msg->flags;

	while(count > 0)
	{
		inval = i2c_inb(pdat);
		if(inval >= 0)
		{
			*temp = inval;
			rdcount++;
		}
		else
		{
			break;
		}

		temp++;
		count--;

		if(rdcount == 1 && (flags & I2C_M_RECV_LEN))
		{
			if (inval <= 0 || inval > 32)
			{
				if(!(flags & I2C_M_NO_RD_ACK))
					acknak(pdat, 0);
				return -1;
			}

			count += inval;
			msg->len += inval;
		}

		if(!(flags & I2C_M_NO_RD_ACK))
		 {
			inval = acknak(pdat, count);
			if(inval < 0)
				return inval;
		 }
	}

	return rdcount;
}

static int bit_do_address(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msg)
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	unsigned char addr;
	int ret, retries;
	retries = nak_ok ? 0 : 3;

	if (flags & I2C_M_TEN)
	{
		addr = 0xf0 | ((msg->addr >> 7) & 0x06);

		ret = try_address(pdat, addr, retries);
		if((ret != 1) && !nak_ok)
		{
			return -1;
		}

		ret = i2c_outb(pdat, msg->addr & 0xff);
		if((ret != 1) && !nak_ok)
		{
			return -1;
		}

		if(flags & I2C_M_RD)
		{
			i2c_repstart(pdat);
			addr |= 0x01;
			ret = try_address(pdat, addr, retries);
			if((ret != 1) && !nak_ok)
			{
				return -1;
			}
		}
	}
	else
	{
		addr = msg->addr << 1;
		if(flags & I2C_M_RD)
			addr |= 1;
		if(flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;
		ret = try_address(pdat, addr, retries);
		if((ret != 1) && !nak_ok)
			return -1;
	}
	return 0;
}

static int i2c_xfer(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num)
{
	struct i2c_gpio_pdata_t * pdat = (struct i2c_gpio_pdata_t *)i2c->priv;
//	return i2c_xfer(pdat, msgs, num);
}

//0 ok , -1 err
static int i2c_probe(struct driver_t * drv,struct driver_info_t * drv_info)
{
	struct i2c_gpio_pdata_t * I2cSim_drv = (struct i2c_gpio_pdata_t *)drv_info->drv_private;
	struct i2c_t * i2c;
	uint16_t i = 0;
	while(i < drv_info->drv_num)
	 {
		i2c = MK_MALLOC(sizeof(struct i2c_t));
		if(!i2c)
		 {
			return -1;
		 }
		i2c->name = I2cSim_drv->node_name;
		i2c->xfer = i2c_xfer;
		i2c->priv = I2cSim_drv;
		kobj_add_regular(drv->kobj, I2cSim_drv->node_name, NULL, NULL, NULL,i2c);
		i++;
		I2cSim_drv++;
	 }
	return 0;
}

static int i2c_remove(struct device_t * dev)
{
	struct i2c_t * i2c = (struct i2c_t *)dev->priv;
	if(i2c)
	 {
		free_device_name(i2c->name);
		MK_FREE(i2c);
	 }
}

static int i2c_suspend(struct device_t * dev){}

static int i2c_resume(struct device_t * dev){}

static struct driver_t i2c_hw = {
	.name		= "i2c",
	.probe		= i2c_probe,
	.remove		= i2c_remove,
	.suspend	= i2c_suspend,
	.resume		= i2c_resume,
};

int i2c_driver_init(void)
{
	return register_driver(&i2c_hw);
}

int i2c_driver_exit(void)
{
	return unregister_driver(&i2c_hw);
}

struct i2c_info_t i2c_drv_info[] = {
	{
        .node_name = "i2c0",
		.Transmit = NULL,
		.Receive = NULL,
    },
};

void init_i2c_driver(void)
{
	struct driver_info_t drv = {
		.drv_name = "i2c",
		.drv_num = ARRAY_SIZE(i2c_drv_info),
		.drv_private = &i2c_drv_info,
	};

	struct driver_t * drv_t;
	drv_t = search_driver(drv.drv_name);
	if(drv_t != NULL)
	 drv_t->probe(drv_t,&drv);
}
