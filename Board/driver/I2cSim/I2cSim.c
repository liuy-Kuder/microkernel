#include "microkernel.h"
#include "stm32f4xx_HAL_gpio.h"
#include "stm32f4xx_ll_utils.h"
#include "I2cSim.h"

#define I2C_SDA(X)     X ? HAL_GPIO_WritePin(pdat->sda_group, pdat->sda_pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(pdat->sda_group, pdat->sda_pin, GPIO_PIN_RESET)
#define I2C_SCL(X)     X ? HAL_GPIO_WritePin(pdat->scl_group, pdat->scl_pin, GPIO_PIN_SET) : HAL_GPIO_WritePin(pdat->scl_group, pdat->scl_pin, GPIO_PIN_RESET)
#define I2C_GET_SDA()  HAL_GPIO_ReadPin(pdat->sda_group, pdat->sda_pin)
#define I2C_DELAY(X)   LL_mDelay(X)

//I2C必须是OD模式

static void i2c_start(struct i2c_gpio_pdata_t * pdat)
{
	I2C_SDA(0);
	I2C_DELAY(pdat->udelay);
	I2C_SCL(0);
}

static void i2c_repstart(struct i2c_gpio_pdata_t * pdat)
{
	I2C_SDA(1);
	I2C_SCL(1);
	I2C_DELAY(pdat->udelay);
	I2C_SDA(0);
	I2C_DELAY(pdat->udelay);
	I2C_SCL(0);
}

static void i2c_stop(struct i2c_gpio_pdata_t * pdat)
{
	I2C_SDA(0);
	I2C_SCL(1);
	I2C_DELAY(pdat->udelay);
	I2C_SDA(1);
	I2C_DELAY(pdat->udelay);
}

static int acknak(struct i2c_gpio_pdata_t * pdat, int is_ack)
{
	if(is_ack)
	  I2C_SDA(0);
	else
	  I2C_SDA(1);
	I2C_DELAY((pdat->udelay + 1) / 2);
	I2C_SCL(1);
	//if(sclhi(pdat) < 0)
	//	return -1;
	I2C_SCL(0);
	return 0;
}

//发送Byte
static unsigned char i2c_outb(struct i2c_gpio_pdata_t * pdat, unsigned char c)
{
	char i;
	int sb;
	char ack;
	//I2C_SCL(0);
	for(i = 7; i >= 0; i--)
	 {
		sb = (c >> i) & 1;
		I2C_SDA(sb);
		I2C_DELAY((pdat->udelay + 1) / 2);
		//if(sclhi(pdat) < 0)
		I2C_SCL(1);
		I2C_DELAY((pdat->udelay + 1) / 2);
		I2C_SCL(0);
	 }
//等待ACK应答
	I2C_SDA(1);
	I2C_DELAY((pdat->udelay + 1) / 2);
	I2C_SCL(1);
	I2C_DELAY((pdat->udelay + 1) / 2);
	//if(sclhi(pdat) < 0)
	//	return -1;
	ack = !I2C_GET_SDA();
	I2C_SCL(0);
	return ack;
}
//读取Byte
static unsigned char i2c_inb(struct i2c_gpio_pdata_t * pdat)
{
	char i;
	unsigned char indata = 0;
	I2C_SDA(1);
	for(i = 0; i < 8; i++)
	 {
		I2C_SCL(1);
		//if(sclhi(pdat) < 0)
		 //return -1;
		indata <<= 1;
		if(I2C_GET_SDA())
			indata |= 0x01;
		I2C_SCL(0);
		I2C_DELAY(i == 7 ? pdat->udelay / 2 : pdat->udelay);
	 }

	return indata;
}

static int try_address(struct i2c_gpio_pdata_t * pdat, unsigned char addr, int retries)
{
	int i, ret = 0;

	for (i = 0; i <= retries; i++)
	{
		ret = i2c_outb(pdat, addr);
		if (ret == 1 || i == retries)
			break;
		i2c_stop(pdat);
		I2C_DELAY(pdat->udelay);
		i2c_start(pdat);
	}

	return ret;
}

//发送字符串
static int sendbytes(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msg)
{
	const unsigned char * temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while(count > 0)
	{
		retval = i2c_outb(pdat, *temp);
		if((retval > 0) || (nak_ok && (retval == 0)))
		 {
			count--;
			temp++;
			wrcount++;
		 }
		else if (retval == 0)
		 {
			return -1;
		 }
		else
		 {
			return retval;
		 }
	}
	return wrcount;
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

int i2c_algo_bit_xfer(struct i2c_gpio_pdata_t * pdat, struct i2c_msg_t * msgs, int num)
{
	struct i2c_msg_t * pmsg;
	int i, ret;
	unsigned short nak_ok;

	i2c_start(pdat);
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

static int i2c_gpio_xfer(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num)
{
	struct i2c_gpio_pdata_t * pdat = (struct i2c_gpio_pdata_t *)i2c->priv;
	return i2c_algo_bit_xfer(pdat, msgs, num);
}

//0 ok , -1 err
static int i2cSim_probe(struct driver_t * drv,struct driver_info_t * drv_info)
{
	struct i2c_gpio_pdata_t * i2cSim_drv = (struct i2c_gpio_pdata_t *)drv_info->drv_private;
	struct i2c_t * i2c;
	uint16_t i = 0;
	while(i < drv_info->drv_num)
	 {
		i2c = MK_MALLOC(sizeof(struct i2c_t));
		if(!i2c)
		 {
			return -1;
		 }
		i2c->name = i2cSim_drv->node_name;
		i2c->xfer = i2c_gpio_xfer;
		i2c->priv = i2cSim_drv;
		kobj_add_regular(drv->kobj, i2cSim_drv->node_name, NULL, NULL, NULL,0,i2c);
		i++;
		i2cSim_drv++;
	 }
	return 0;
}

static int i2cSim_remove(struct device_t * dev)
{
	struct i2c_t * i2c = (struct i2c_t *)dev->priv;
	if(i2c)
	 {
		free_device_name(i2c->name);
		MK_FREE(i2c);
        return 0;
	 }
    return -1;
}

static int i2cSim_suspend(struct device_t * dev){return 0;}

static int i2cSim_resume(struct device_t * dev){return 0;}

static struct driver_t i2cSim = {
	.name		= "i2cSim",
	.probe		= i2cSim_probe,
	.remove		= i2cSim_remove,
	.suspend	= i2cSim_suspend,
	.resume		= i2cSim_resume,
};

int I2cSim_driver_init(void)
{
	return register_driver(&i2cSim);
}

int I2cSim_driver_exit(void)
{
	return unregister_driver(&i2cSim);
}

struct i2c_gpio_pdata_t i2cSim_drv_info[] = {
	{
        .node_name = "i2cSim0",
		.scl_pin = LL_GPIO_PIN_9,
        .scl_group = GPIOB,
		.sda_pin = LL_GPIO_PIN_8,
        .sda_group = GPIOB,
		.udelay = 5,
    },
};

void init_I2cSim_driver(void)
{
	struct driver_info_t drv = {
		.drv_name = "i2cSim",
		.drv_num = ARRAY_SIZE(i2cSim_drv_info),
		.drv_private = &i2cSim_drv_info,
	};

	struct driver_t * drv_t;
	drv_t = search_driver(drv.drv_name);
	if(drv_t != NULL)
	 drv_t->probe(drv_t,&drv);
}
