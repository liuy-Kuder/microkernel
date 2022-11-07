/********************************************************************
*
*文件名称：kobj.c
*内容摘要：MicroKernel核心功能
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.09.12
*其他说明: none
*
**********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "microkernel.h"

static struct kobj_t * __kobj_root = NULL;

/********************************************************************
*                      功能函数
*功能描述：分配一段内存,然后用传递的参数进行初始化,并返回分配的kobj节点
*输入参数：
*		name: 名称
*		type: 文件 或 节点
*		read: 读节点
*		write: 写节点
*		ioctl: IO操作
*		size: 该节点可寻址空间
*		priv: 私有数据块
*返回值：分配的kobj节点
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
static struct kobj_t * __kobj_alloc(const char * name, enum kobj_type_t type,\
									kobj_read_t read, kobj_write_t write, kobj_ioctl_t ioctl, size_t size, void * priv)
{
	struct kobj_t * kobj;

	if(!name)
		return NULL;//名称不为空

	kobj = MK_MALLOC(sizeof(struct kobj_t));//malloc(sizeof(struct kobj_t));//分配字节内存
	if(!kobj)
		return NULL;//分配失败

	kobj->name = strdup(name);//拷贝
	kobj->type = type;//类型
	kobj->parent = kobj;//父类
	init_list_head(&kobj->entry);//初始化列表
	init_list_head(&kobj->children);
	kobj->read = read;
	kobj->write = write;
	kobj->priv = priv;
	kobj->ioctl = ioctl;
	kobj->size = size;
	return kobj;
}
/********************************************************************
*                      功能函数
*功能描述：在获取根节点时，如果不存在，则会自动创建一个名为kobj的根节点，
*		  该节点为全局静态变量，同时也是一个sysfs的顶层目录节点，
*		  在mount文件系统时会挂载到sys目录
*输入参数：无
*返回值：kobj根节点
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct kobj_t * kobj_get_root(void)
{
	if(!__kobj_root)
		__kobj_root = kobj_alloc_directory("kobj");
	return __kobj_root;
}

/********************************************************************
*                      功能函数
*功能描述： 从一个父节点中搜索一个名为name的子节点。
*输入参数：
*		parent: 父节点
*		name:名称
*返回值：name对应的位置
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name)
{
	struct kobj_t * pos, * n;//pos位置,临时变量n

	if(!parent)//参数不为空
	 return NULL;

	if(parent->type != KOBJ_TYPE_DIR)//类型不是目录
		return NULL;

	if(!name)//名字不为空
		return NULL;

	list_for_each_entry_safe(pos, n, struct kobj_t,&(parent->children), entry)//循环查找
	{
		if(strcmp(pos->name, name) == 0)
			return pos;
	}

	return NULL;
}

/********************************************************************
*                      功能函数
*功能描述：在父节点下未搜索到名为name的子节点时,自动创建一个子节点,该节点为目录类型
*输入参数：
*		parent: 父节点 
*		name: 名称
*返回值：子节点
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name)
{
	struct kobj_t * kobj;

	if(!parent)
		return NULL;

	if(parent->type != KOBJ_TYPE_DIR)
		return NULL;//类型不是目录

	if(!name)
		return NULL;

	kobj = kobj_search(parent, name);
	if(kobj == NULL)
	{
		kobj = kobj_alloc_directory(name);
		if(!kobj)
			return NULL;

		if(!kobj_add(parent, kobj))
		{
			kobj_free(kobj);
			return NULL;
		}
	}
	else if(kobj->type != KOBJ_TYPE_DIR)
	{
		return NULL;
	}

	return kobj;
}

/********************************************************************
*                      功能函数
*功能描述： 其为内部函数__kobj_alloc的二次封装，快速分配指定类型的目录节点
*输入参数：name:名称
*返回值：kobj
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct kobj_t * kobj_alloc_directory(const char * name)
{
	return __kobj_alloc(name, KOBJ_TYPE_DIR, NULL, NULL, NULL, 0, NULL);
}

/********************************************************************
*                      功能函数
*功能描述： 其为内部函数__kobj_alloc的二次封装，快速分配指定类型的文件节点
*输入参数：
*		name: 名称
*		read: 读节点
*		write: 写节点
*		ioctl: IO操作
*		size: 该节点可寻址空间
*		priv: 私有数据块
*返回值：kobj
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, kobj_ioctl_t ioctl, size_t size, void * priv)
{
	return __kobj_alloc(name, KOBJ_TYPE_REG, read, write, ioctl, size, priv);
}

/********************************************************************
*                      功能函数
*功能描述： kobj动态分配的内存进行回收
*输入参数： kobj
*返回值：FALSE: 失败、TRUE: 成功
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_free(struct kobj_t * kobj)
{
	if(!kobj)
		return FALSE;
	MK_FREE(kobj->name);
	MK_FREE(kobj);
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述： 将一个节点,可以是目录节点或文件节点,添加至父目录节点
*输入参数：
*		parent: 父节点
*		kobj: obj
*返回值：成功返回真,否则返回假
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_add(struct kobj_t * parent, struct kobj_t * kobj)
{
	if(!parent)
		return FALSE;

	if(parent->type != KOBJ_TYPE_DIR)
		return FALSE;

	if(!kobj)
		return FALSE;

	if(kobj_search(parent, kobj->name))
		return FALSE;

	spin_lock_irq();
	kobj->parent = parent;
	list_add_tail(&kobj->entry, &parent->children);
	spin_unlock_irq();

	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述： 从一个父目录节点中删除一个子节点
*输入参数：
*		parent: 父节点
*		kobj : obj
*返回值：成功返回真,否则返回假
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_remove(struct kobj_t * parent, struct kobj_t * kobj)
{
	struct kobj_t * pos, * n;

	if(!parent)
		return FALSE;

	if(parent->type != KOBJ_TYPE_DIR)
		return FALSE;

	if(!kobj)
		return FALSE;

	list_for_each_entry_safe(pos, n, struct kobj_t, &(parent->children), entry)
	 {
		if(pos == kobj)
		 {
			spin_lock_irq();
			pos->parent = pos;
			list_del(&(pos->entry));
			spin_unlock_irq();
			return TRUE;
		 }
	 }

	return FALSE;
}

/********************************************************************
*                      功能函数
*功能描述：此接口实现的目的是为了快速添加子节点，其根据所传递的参数，
*			自动创建节点并添加到父节点上。
*输入参数：
*		parent: 父节点
*		name: 名称
*返回值：成功返回真,否则返回假
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_add_directory(struct kobj_t * parent, const char * name)
{
	struct kobj_t * kobj;

	if(!parent)
		return FALSE;

	if(parent->type != KOBJ_TYPE_DIR)
		return FALSE;

	if(!name)
		return FALSE;

	if(kobj_search(parent, name))
		return FALSE;

	kobj = kobj_alloc_directory(name);
	if(!kobj)
		return FALSE;

	if(!kobj_add(parent, kobj))
		kobj_free(kobj);

	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：添加子文件节点
*输入参数：
*		parent: 父节点
*		name: 名称
*		read: 读节点
*		write: 写节点
*		ioctl: IO操作
*		priv: 私有数据块
*返回值：成功返回真,否则返回假
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_add_regular(struct kobj_t * parent, const char * name, \
						 kobj_read_t read, kobj_write_t write, kobj_ioctl_t ioctl,size_t size, void * priv)
{
	struct kobj_t * kobj;

	if(!parent)
		return FALSE;

	if(parent->type != KOBJ_TYPE_DIR)
		return FALSE;

	if(!name)
		return FALSE;

	if(kobj_search(parent, name))
		return FALSE;

	kobj = kobj_alloc_regular(name, read, write, ioctl, size, priv);
	if(!kobj)
		return FALSE;

	if(!kobj_add(parent, kobj))
		kobj_free(kobj);

	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：此接口会将自身及其所有子节点以递归方式删除
*输入参数：节点
*返回值：成功返回真,否则返回假
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
uint8_t kobj_remove_self(struct kobj_t * kobj)
{
	struct kobj_t * parent;
	struct kobj_t * pos, * n;
	uint8_t ret;

	if(!kobj)
		return FALSE;

	if(kobj->type == KOBJ_TYPE_DIR)
	{
		list_for_each_entry_safe(pos, n, struct kobj_t,&(kobj->children), entry)
		{
			kobj_remove_self(pos);
		}
	}

	parent = kobj->parent;
	if(parent && (parent != kobj))
	{
		ret = kobj_remove(parent, kobj);
		if(ret)
			kobj_free(kobj);
		return ret;
	}

	kobj_free(kobj);
	return TRUE;
}

/********************************************************************
*                      功能函数
*功能描述：根据字符串动态分配hash值
*输入参数：字符串
*返回值：返回value
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.19     1.0        刘杨
**********************************************************************/
uint32_t shash(const char * s)
{
	uint32_t v = 5381;
	if(s)
	{
		while(*s)
			v = (v << 5) + v + (*s++);
	}
	//MK_LOG_TRACE("string: %s,value: %d\n",s,v);
	return v;
}
