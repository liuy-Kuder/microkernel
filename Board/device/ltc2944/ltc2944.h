#ifndef  __LTC2944_H_
#define  __LTC2944_H_

#include "stm32f4xx_hal_gpio.h"

#define LTC2943_STATUS_REG                   0x00
#define LTC2943_CONTROL_REG                  0x01
#define LTC2943_ACCUM_CHARGE_REG             0x02
#define LTC2943_CHARGE_THRESH_H_REG          0x04
#define LTC2943_CHARGE_THRESH_L_REG          0x06
#define LTC2943_VOLTAGE_REG                  0x08
#define LTC2943_VOLTAGE_THRESH_H_REG         0x0A
#define LTC2943_VOLTAGE_THRESH_L_REG         0x0C
#define LTC2943_CURRENT_REG                  0x0E
#define LTC2943_CURRENT_THRESH_H_REG         0x10
#define LTC2943_CURRENT_THRESH_L_REG         0x12
#define LTC2943_TEMPERATURE_REG              0x14
#define LTC2943_TEMPERATURE_THRESH_H_REG     0x16
#define LTC2943_TEMPERATURE_THRESH_L_REG     0x17

#define ADC_AUTOMATIC_MODE			 0x03//持续开始转换电压、电流、温度
#define ADC_SCAN_MODE				 0x02//每10S转换一次电压、电流、温度
#define ADC_MANUAL_MODE				 0x01//单次转换电压、电流、温度
#define ADC_SLEEP_MODE				 0x00//睡眠
#define ADC_MODE_POS  				 6
#define ADC_MODE  					 ADC_AUTOMATIC_MODE << ADC_MODE_POS

#define FACTOR_M_1                   0x00
#define FACTOR_M_4                   0x01
#define FACTOR_M_16                  0x02
#define FACTOR_M_64                  0x03
#define FACTOR_M_256                 0x04
#define FACTOR_M_1024                0x05
#define FACTOR_M_4096                0x06
#define SETS_COULOMB_FACTOR_M_POS    3
#define SETS_COULOMB_PRESCA_FACTOR_M  FACTOR_M_4096 << SETS_COULOMB_FACTOR_M_POS

#define ALRET_MODE				 	 0x02//报警使能，PIN脚作为逻辑输出
#define CHARGE_COMPLETE_MODE		 0x01//充电完成模式，PIN脚作为逻辑输入，并接受充电完成反转信号将 LTC2943_ACCUM_CHARGE_REG 设置为0xFFFF
#define ALRET_DISABLE_MODE		 	 0x00//ALCC引脚关闭
#define ALRET_CONFIG_POS		 	 1
#define ALCC_CONFIG_MODE		 	 ALRET_MODE << ALRET_CONFIG_POS

#define SHUTDOWN_ENABLE	 	 	 	 0x01//将关闭芯片中模拟电路，此时没有任何测量功能，仅保持I2C通信模块
#define SHUTDOWN_DISABLE	 	 	 0x00//正常运行
#define SHUTDOWN_MODE_POS	 	 	 0
#define SHUTDOWN_MODE	 	 	 	 SHUTDOWN_DISABLE << SHUTDOWN_MODE_POS

//ioctl
#define LTC2944_GET_ALCC_PIN_STATE	 	 0x00


struct device_priv_t
{
	uint16_t addr;
	struct kobj_t * bus;
	uint32_t alcc_pin;
	GPIO_TypeDef *alcc_group;
	GPIO_PinState (*read_alcc_pin)(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
};

struct device_ltc2944_info_t{
	char * device_name;
	int device_addr;
	char * node_name;
    uint32_t alcc_pin;
	GPIO_TypeDef *alcc_group;
	GPIO_PinState (*read_alcc_pin)(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
};

struct device_t * init_ltc2944_device(void);

#endif
