#ifndef __MK_DEVICE_H_
#define __MK_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum device_type_t {
	DEVICE_TYPE_ADC				= 0,
	DEVICE_TYPE_AUDIO			= 1,
	DEVICE_TYPE_BATTERY			= 2,
	DEVICE_TYPE_BLOCK			= 3,
	DEVICE_TYPE_BUZZER			= 4,
	DEVICE_TYPE_CAMERA			= 5,
	DEVICE_TYPE_CLK				= 6,
	DEVICE_TYPE_CLOCKEVENT		= 7,
	DEVICE_TYPE_CLOCKSOURCE		= 8,
	DEVICE_TYPE_COMPASS			= 9,
	DEVICE_TYPE_CONSOLE			= 10,
	DEVICE_TYPE_DAC				= 11,
	DEVICE_TYPE_DMACHIP			= 12,
	DEVICE_TYPE_FRAMEBUFFER		= 13,
	DEVICE_TYPE_GMETER			= 14,
	DEVICE_TYPE_GNSS			= 15,
	DEVICE_TYPE_GPIOCHIP		= 16,
	DEVICE_TYPE_GYROSCOPE		= 17,
	DEVICE_TYPE_HYGROMETER		= 18,
	DEVICE_TYPE_I2C				= 19,
	DEVICE_TYPE_INPUT			= 20,
	DEVICE_TYPE_IRQCHIP			= 21,
	DEVICE_TYPE_LED				= 22,
	DEVICE_TYPE_LEDSTRIP		= 23,
	DEVICE_TYPE_LEDTRIGGER		= 24,
	DEVICE_TYPE_LIGHT			= 25,
	DEVICE_TYPE_MOTOR			= 26,
	DEVICE_TYPE_NET				= 27,
	DEVICE_TYPE_NVMEM			= 28,
	DEVICE_TYPE_PRESSURE		= 29,
	DEVICE_TYPE_PROXIMITY		= 30,
	DEVICE_TYPE_PWM				= 31,
	DEVICE_TYPE_REGULATOR		= 32,
	DEVICE_TYPE_RESETCHIP		= 33,
	DEVICE_TYPE_RNG				= 34,
	DEVICE_TYPE_RTC				= 35,
	DEVICE_TYPE_SDHCI			= 36,
	DEVICE_TYPE_SERVO			= 37,
	DEVICE_TYPE_SPI				= 38,
	DEVICE_TYPE_STEPPER			= 39,
	DEVICE_TYPE_THERMOMETER		= 40,
	DEVICE_TYPE_UART			= 41,
	DEVICE_TYPE_VIBRATOR		= 42,
	DEVICE_TYPE_WATCHDOG		= 43,

	DEVICE_TYPE_MAX_COUNT		= 44,
};

struct driver_t;
struct device_t
{
	struct kobj_t * kobj;
	struct list_head list;
	struct list_head head;
	struct hlist_node node;

	char * name;
	enum device_type_t type;
	struct driver_t * driver;
	void * priv;
};

struct device_info_t{
	char * device_name;
	char * node_name;
	char * attribute;
};

struct register_info_t{
	char * driver_name;
	uint8_t device_num;
	void * device_private;
};

extern struct list_head __device_list;
extern struct list_head __device_head[DEVICE_TYPE_MAX_COUNT];

char * alloc_device_name(const char * name, int id);
int free_device_name(char * name);
struct device_t * search_device(const char * name, enum device_type_t type);
struct device_t * search_first_device(enum device_type_t type);
uint16_t GetDeviceNum(void);
uint8_t register_device(struct device_t * dev);
uint8_t unregister_device(struct device_t * dev);
void remove_device(struct device_t * dev);
void device_pure_init(void);
#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_H__ */
