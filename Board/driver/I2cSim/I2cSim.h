#ifndef  __I2CSIM_H_
#define  __I2CSIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "microkernel.h"
#include "stm32f4xx_ll_gpio.h"

enum {
	I2C_M_TEN			= 0x0010,/* this is a ten bit chip address */
	I2C_M_RD			= 0x0001,/* read data, from slave to master */
	I2C_M_STOP			= 0x8000,/* if I2C_FUNC_PROTOCOL_MANGLING */
	I2C_M_NOSTART		= 0x4000,/* if I2C_FUNC_NOSTART */
	I2C_M_REV_DIR_ADDR	= 0x2000,/* if I2C_FUNC_PROTOCOL_MANGLING */
	I2C_M_IGNORE_NAK	= 0x1000,/* if I2C_FUNC_PROTOCOL_MANGLING */
	I2C_M_NO_RD_ACK		= 0x0800,/* if I2C_FUNC_PROTOCOL_MANGLING */
	I2C_M_RECV_LEN		= 0x0400,/* length will be first received byte */
};

struct i2c_msg_t {
	int addr;
	int flags;
	int len;
	void * buf;
};

struct i2c_t{
	/* The i2c bus name */
	char * name;
	/* Master xfer */
	int (*xfer)(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num);
	/* Private data */
	void * priv;
};

struct i2c_gpio_pdata_t {
	char* node_name;
	uint32_t scl_pin;
	GPIO_TypeDef *scl_group;
    uint32_t sda_pin;
	GPIO_TypeDef *sda_group;
	int udelay;
};

int I2cSim_driver_init(void);
int I2cSim_driver_exit(void);
void init_I2cSim_driver(void);

#ifdef __cplusplus
}
#endif

#endif
