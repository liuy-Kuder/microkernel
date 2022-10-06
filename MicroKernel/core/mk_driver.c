/********************************************************************
*
*文件名称：mk_driver.c
*内容摘要：提供总线读写平台
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.09.12
*其他说明: none
*
**********************************************************************/
#include "microkernel.h"
#include "../log/mk_log.h"

static struct hlist_head __driver_hash[CONFIG_DRIVER_HASH_SIZE];

static struct hlist_head * driver_hash(const char * name)
{
	return &__driver_hash[shash(name) % ARRAY_SIZE(__driver_hash)];
}

/********************************************************************
*                      功能函数
*功能描述：寻找驱动kobj
*输入参数：无
*返回值：struct kobj_t
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static struct kobj_t * search_class_driver_kobj(void)
{
	return kobj_search_directory_with_create(kobj_get_root(), "driver");
}

/********************************************************************
*                      功能函数
*功能描述：根据名字寻找驱动
*输入参数：无
*返回值：真：驱动地址、假：NULL
*其他说明：NULL or POS
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct driver_t * search_driver(const char * name)
{
	struct driver_t * pos;
	struct hlist_node * n;

	if(!name)
		return NULL;

    hlist_for_each_entry_safe(pos, struct driver_t, n, driver_hash(name), node)
	{
		if(strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}

/********************************************************************
*                      功能函数
*功能描述：注册驱动
*输入参数：无
*返回值：false：失败 true：成功
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t register_driver(struct driver_t * drv)
{
	char Ibuf[128];

	if(!drv || !drv->name)//空或无名字、返回假
	 {
		sprintf(Ibuf,"%s register driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	if(!drv->probe || !drv->remove)//无管道或无移除、返回假
	 {
		sprintf(Ibuf,"%s register driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	if(!drv->suspend || !drv->resume)//无暂停或无释放、返回假
	 {
		sprintf(Ibuf,"%s register driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	if(search_driver(drv->name))//寻找驱动名，如果相同则返回假
	 {
		sprintf(Ibuf,"%s register driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	drv->kobj = kobj_alloc_directory(drv->name);
	kobj_add(search_class_driver_kobj(), drv->kobj);

	spin_lock_irq();//关中断
	init_hlist_node(&drv->node);
	hlist_add_head(&drv->node, driver_hash(drv->name));
	spin_unlock_irq();//开中断
	sprintf(Ibuf,"%s register driver success!",drv->name);
	MK_LOG_INFO(Ibuf);
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：卸载驱动
*输入参数：无
*返回值：false：失败 true：成功
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t unregister_driver(struct driver_t * drv)
{
	char Ibuf[128];
	if(!drv || !drv->name)
	 {
		sprintf(Ibuf,"%s unregister driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	if(hlist_unhashed(&drv->node))
	 {
		sprintf(Ibuf,"%s unregister driver fail!",drv->name);
		MK_LOG_ERROR(Ibuf);
		return FALSE;
	 }

	spin_lock_irq();
	hlist_del(&drv->node);
	spin_unlock_irq();
	kobj_remove_self(drv->kobj);
	sprintf(Ibuf,"%s unregister driver success!",drv->name);
	MK_LOG_INFO(Ibuf);
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：初始化设备列表
*输入参数：无
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
void driver_pure_init(void)
{
	uint16_t i;
	for(i = 0; i < ARRAY_SIZE(__driver_hash); i++)
		init_hlist_head(&__driver_hash[i]);
}
