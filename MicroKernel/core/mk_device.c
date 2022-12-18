/********************************************************************
*
*文件名称：mk_device.c
*内容摘要：提供设备管理平台
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.09.12
*其他说明: none
*
**********************************************************************/

#include "microkernel.h"
#include "mk_setup.h"
#include "../log/mk_log.h"

struct list_head __device_list;
struct list_head __device_head[DEVICE_TYPE_MAX_COUNT];
static struct hlist_head __device_hash[CONFIG_DEVICE_HASH_SIZE];

static struct hlist_head * device_hash(const char * name)
{
	return &__device_hash[shash(name) % ARRAY_SIZE(__device_hash)];
}

/********************************************************************
*                      功能函数
*功能描述： 搜索device下的name目录，没有就创建
*输入参数：dev
*返回值：kobj节点
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static struct kobj_t * search_device_kobj(struct device_t * dev)
{
	struct kobj_t * kdevice;
	char * name;

	if(!dev || !dev->kobj)
	  return NULL;

	kdevice = kobj_search_directory_with_create(kobj_get_root(), "device");
	if(!kdevice)
	  return NULL;

	switch(dev->type)
	  {
		case DEVICE_TYPE_ADC:
			name = "adc";
			break;
		case DEVICE_TYPE_AUDIO:
			name = "audio";
			break;
		case DEVICE_TYPE_BATTERY:
			name = "battery";
			break;
		case DEVICE_TYPE_BLOCK:
			name = "block";
			break;
		case DEVICE_TYPE_BUZZER:
			name = "buzzer";
			break;
		case DEVICE_TYPE_CAMERA:
			name = "camera";
			break;
		case DEVICE_TYPE_CLK:
			name = "clk";
			break;
		case DEVICE_TYPE_CLOCKEVENT:
			name = "clockevent";
			break;
		case DEVICE_TYPE_CLOCKSOURCE:
			name = "clocksource";
			break;
		case DEVICE_TYPE_COMPASS:
			name = "compass";
			break;
		case DEVICE_TYPE_CONSOLE:
			name = "console";
			break;
		case DEVICE_TYPE_DAC:
			name = "dac";
			break;
		case DEVICE_TYPE_DMACHIP:
			name = "dmachip";
			break;
		case DEVICE_TYPE_FRAMEBUFFER:
			name = "framebuffer";
			break;
		case DEVICE_TYPE_GMETER:
			name = "gmeter";
			break;
		case DEVICE_TYPE_GNSS:
			name = "gnss";
			break;
		case DEVICE_TYPE_GPIOCHIP:
			name = "gpiochip";
			break;
		case DEVICE_TYPE_GYROSCOPE:
			name = "gyroscope";
			break;
		case DEVICE_TYPE_HYGROMETER:
			name = "hygrometer";
			break;
		case DEVICE_TYPE_I2C:
			name = "i2c";
			break;
		case DEVICE_TYPE_INPUT:
			name = "input";
			break;
		case DEVICE_TYPE_IRQCHIP:
			name = "irqchip";
			break;
		case DEVICE_TYPE_LED:
			name = "led";
			break;
		case DEVICE_TYPE_LEDSTRIP:
			name = "ledstrip";
			break;
		case DEVICE_TYPE_LEDTRIGGER:
			name = "ledtrigger";
			break;
		case DEVICE_TYPE_LIGHT:
			name = "light";
			break;
		case DEVICE_TYPE_MOTOR:
			name = "motor";
			break;
		case DEVICE_TYPE_NET:
			name = "net";
			break;
		case DEVICE_TYPE_NVMEM:
			name = "nvmem";
			break;
		case DEVICE_TYPE_PRESSURE:
			name = "pressure";
			break;
		case DEVICE_TYPE_PROXIMITY:
			name = "proximity";
			break;
		case DEVICE_TYPE_PWM:
			name = "pwm";
			break;
		case DEVICE_TYPE_REGULATOR:
			name = "regulator";
			break;
		case DEVICE_TYPE_RESETCHIP:
			name = "resetchip";
			break;
		case DEVICE_TYPE_RNG:
			name = "rng";
			break;
		case DEVICE_TYPE_RTC:
			name = "rtc";
			break;
		case DEVICE_TYPE_SDHCI:
			name = "sdhci";
			break;
		case DEVICE_TYPE_SERVO:
			name = "servo";
			break;
		case DEVICE_TYPE_SPI:
			name = "spi";
			break;
		case DEVICE_TYPE_STEPPER:
			name = "stepper";
			break;
		case DEVICE_TYPE_THERMOMETER:
			name = "thermometer";
			break;
		case DEVICE_TYPE_UART:
			name = "uart";
			break;
		case DEVICE_TYPE_VIBRATOR:
			name = "vibrator";
			break;
		case DEVICE_TYPE_WATCHDOG:
			name = "watchdog";
			break;
		default:
			return NULL;
	  }

	return kobj_search_directory_with_create(kdevice, (const char *)name);
}

/********************************************************************
*                      功能函数
*功能描述：设备挂起
*输入参数：device_t
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
static void suspend_device(struct device_t * dev)
{
	if(dev)
	 {
		if(dev->driver && dev->driver->suspend)
		  dev->driver->suspend(dev);
		MK_LOG_TRACE("%s suspend device success!\n",dev->name);
	 }
}

/********************************************************************
*                      功能函数
*功能描述：设备恢复
*输入参数：device_t
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
static void resume_device(struct device_t * dev)
{
	if(dev)
	 {
		if(dev->driver && dev->driver->resume)
		  dev->driver->resume(dev);
		MK_LOG_TRACE("%s resume device success!\n",dev->name);
	 }
}

/********************************************************************
*                      功能函数
*功能描述： 驱动写暂停
*输入参数：
*		kobj：对象
*		buf：驱动名
*		size：驱动名长度
*返回值：长度
*其他说明：
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static int64_t device_write_suspend(struct kobj_t * kobj, int64_t offset, void * buf, int64_t size)
{
	int ret = 0;
	struct device_t * dev = (struct device_t *)kobj->priv;

	if(strncmp(buf, dev->name, size) == 0)
		suspend_device(dev);
	return ret;
}

/********************************************************************
*                      功能函数
*功能描述： 驱动写恢复
*输入参数：
*		kobj：对象
*		buf：驱动名
*		size：驱动名长度
*返回值：长度
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static int64_t device_write_resume(struct kobj_t * kobj,int64_t offset, void * buf, int64_t size)
{
	int ret = 0;
	struct device_t * dev = (struct device_t *)kobj->priv;

	if(strncmp(buf, dev->name, size) == 0)
		resume_device(dev);
	return ret;
}

/********************************************************************
*                      功能函数
*功能描述：驱动是否存在
*输入参数：名字
*返回值：存在返回true,否则返回false
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static uint8_t device_exist(const char * name)
{
	struct device_t * pos;
	struct hlist_node * n;

	hlist_for_each_entry_safe(pos, struct device_t, n, device_hash(name), node)
	 {
		if(strcmp(pos->name, name) == 0)
		  return TRUE;
	 }
	return FALSE;
}

/********************************************************************
*                      功能函数
*功能描述：自动分配驱动名称,如果当前设备的ID已被注册,则自动增加,重新分配
*输入参数：设备名字,ID
*返回值：返回分配好的名称地址
*其他说明：需要free释放这个返回值
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
char * alloc_device_name(const char * name, uint16_t id)
{
	char buf[256];

	do {
		snprintf(buf, sizeof(buf), "%s.%d", name, id++);
	} while(device_exist(buf));

	return strdup(buf);
}

/********************************************************************
*                      功能函数
*功能描述：释放设备的名称
*输入参数：名字
*返回值：FALSE or TRUE
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
*2022.12.17    1.1        刘杨         修改返回值
**********************************************************************/
uint8_t free_device_name(char * name)
{
	if(name)
	 {
		MK_FREE(name);
		return TRUE;
	 }
	else
	 return FALSE;
}

/********************************************************************
*                      功能函数
*功能描述：寻找设备
*输入参数：name: 名字
*         type: 设备类型
*返回值：设备地址
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
struct device_t * search_device(const char * name, enum device_type_t type)
{
	struct device_t * pos;
	struct hlist_node * n;

	if(!name)
	  return NULL;

	hlist_for_each_entry_safe(pos, struct device_t, n, device_hash(name), node)
	 {
		if((pos->type == type) && (strcmp(pos->name, name) == 0))
		  return pos;
	 }
	return NULL;
}

/********************************************************************
*                      功能函数
*功能描述：寻找指定类型的第一个设备
*输入参数：type: 设备类型
*返回值：设备地址
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
struct device_t * search_first_device(enum device_type_t type)
{
	if(type >= ARRAY_SIZE(__device_head))
	  return NULL;

	void * pos = list_first_entry_or_null(&__device_head[type], struct device_t, head);
	return (struct device_t *) pos;
}

/********************************************************************
*                      功能函数
*功能描述：device数量累加
*输入参数：无
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.6     1.0        刘杨
**********************************************************************/
static uint16_t decive_cnt = 0;
static void device_inc(void)
{
	decive_cnt++;
}

/********************************************************************
*                      功能函数
*功能描述：device数量减少
*输入参数：无
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.6     1.0        刘杨
**********************************************************************/
static void device_dec(void)
{
	decive_cnt--;
}

/********************************************************************
*                      功能函数
*功能描述：获得device的数量
*输入参数：无
*返回值：device数量
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.6     1.0        刘杨
**********************************************************************/
uint16_t GetDeviceNum(void)
{
	return decive_cnt;
}

/********************************************************************
*                      功能函数
*功能描述：注册设备
*输入参数：device_t
*返回值：成功返回true,失败返回false
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
uint8_t register_device(struct device_t * dev)
{
	if(!dev || !dev->name)
	 {
		MK_LOG_ERROR("%s register device null or name null!\n",dev->name);
		return FALSE;
	 }
		
    if(dev->type >= ARRAY_SIZE(__device_head))
	 {
		MK_LOG_ERROR("%s register device type over index!\n",dev->name);
		return FALSE;
	 }

	if(device_exist(dev->name))
	 {
		MK_LOG_ERROR("%s register device have existing!\n",dev->name);
		return FALSE;
	 }

	kobj_add_regular(dev->kobj, "suspend", NULL, device_write_suspend,NULL, 0, dev);
	kobj_add_regular(dev->kobj, "resume", NULL, device_write_resume,NULL, 0, dev);
	kobj_add(search_device_kobj(dev), dev->kobj);
	spin_lock_irq();
	init_list_head(&dev->list);
	list_add_tail(&dev->list, &__device_list);
	init_list_head(&dev->head);
	list_add_tail(&dev->head, &__device_head[dev->type]);
	init_hlist_node(&dev->node);
	hlist_add_head(&dev->node, device_hash(dev->name));
	spin_unlock_irq();
	MK_LOG_TRACE("%s device install success!\n",dev->name);
	device_inc();
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：卸载设备
*输入参数：device_t
*返回值：成功返回true,失败返回false
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
uint8_t unregister_device(struct device_t * dev)
{
	if(!dev || !dev->name)
	 {
		MK_LOG_ERROR("%s unregister device null or name null!\n",dev->name);
		return FALSE;
	 }

    if(dev->type >= ARRAY_SIZE(__device_head))
	 {
		MK_LOG_ERROR("%s unregister device type over index!\n",dev->name);
		return FALSE;
	 }

	if(hlist_unhashed(&dev->node))
	 {
		MK_LOG_ERROR("%s unregister device fail!\n",dev->name);
		return FALSE;
	 }

	spin_lock_irq();
	list_del(&dev->list);
	list_del(&dev->head);
	hlist_del(&dev->node);
	spin_unlock_irq();
	kobj_remove(search_device_kobj(dev), dev->kobj);
	MK_LOG_TRACE("%s unregister device success!\n",dev->name);
	device_dec();
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：初始化设备池
*输入参数：无
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12      1.0       刘杨
**********************************************************************/
void device_pure_init(void)
{
	uint16_t i;

	init_list_head(&__device_list);
	for(i = 0; i < ARRAY_SIZE(__device_head); i++)
	  init_list_head(&__device_head[i]);
	for(i = 0; i < ARRAY_SIZE(__device_hash); i++)
	  init_hlist_head(&__device_hash[i]);
}
