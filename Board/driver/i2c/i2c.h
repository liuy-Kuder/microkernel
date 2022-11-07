#ifndef  __I2C_H_
#define  __I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "microkernel.h"
#include "stm32f4xx_hal_i2c.h"

enum {
	I2C_M_TEN			= 0x0010,
	I2C_M_RD			= 0x0001,
	I2C_M_STOP			= 0x8000,
	I2C_M_NOSTART		= 0x4000,
	I2C_M_REV_DIR_ADDR	= 0x2000,
	I2C_M_IGNORE_NAK	= 0x1000,
	I2C_M_NO_RD_ACK		= 0x0800,
	I2C_M_RECV_LEN		= 0x0400,
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

struct i2c_info_t{
	char* node_name;
	HAL_StatusTypeDef (*Transmit)(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	HAL_StatusTypeDef (*Receive)(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

};

int i2c_driver_init(void);
int i2c_driver_exit(void);
void init_i2c_driver(void);

#ifdef __cplusplus
}
#endif

#endif
