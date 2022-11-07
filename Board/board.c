#include "main.h"
#include "cmsis_os.h"
#include "microkernel.h"
#include "ltc2944.h"

//板子外设初始化
struct i2c_gpio_pdata_t I2cSim_drv_info[] = {
	{
        .node_name = "i2cSim0",
		.scl_pin = LL_GPIO_PIN_9,
        .scl_group = GPIOB,
		.sda_pin = LL_GPIO_PIN_8,
        .sda_group = GPIOB,
		.udelay = 5,
    },
};

/*
1、注册总线
2、初始化总线
3、注册驱动
*/
int board_init(void)
{
    int i;
    //1、注册总线
    I2cSim_driver_init();

    //2、初始化总线
	struct driver_info_t drv = {
		.drv_name = "i2cSim",
		.drv_num = ARRAY_SIZE(I2cSim_drv_info),
		.drv_private = &I2cSim_drv_info,
	};

    for(i = 0; i < 1; i++)
     {
        struct driver_t * drv_t;
        drv_t = search_driver(drv.drv_name);
        if(drv_t != NULL)
         drv_t->probe(drv_t,&drv);
     }

    //3、注册驱动
    init_ltc2944_device();

}

