/********************************************************************
*
*文件名称：sysfs.c
*内容摘要：提供文件系统所需的驱动、总线的底层调用
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.10.3
*其他说明: none
*
**********************************************************************/

#include "microkernel.h"
#include "sysfs.h"
#include "vfs.h"
#include "rtos.h"

/********************************************************************
*                      功能函数
*功能描述： 系统挂载
*输入参数：dev
*返回值：kobj节点
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int sys_mount(struct vfs_mount_t * m, const char * dev)
{
	if(dev)
		return -1;

	m->m_flags |= MOUNT_RW;
	m->m_root->v_data = (void *)kobj_get_root();
	m->m_data = NULL;
	return 0;
}

/********************************************************************
*                      功能函数
*功能描述： 系统卸载
*输入参数：vfs_mount_t 结构参数
*返回值：OK
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int sys_unmount(struct vfs_mount_t * m)
{
	m->m_data = NULL;
	return 0;
}

/********************************************************************
*                      功能函数
*功能描述： 读取
*输入参数：n: kobj节点信息
*		  off: 偏移地址
*		  buf: 接收缓冲区
*		  len: 接收长度
*返回值：接收长度
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int64_t sys_read(struct vfs_node_t * n, int64_t offset, void * buf, int64_t len)
{
	struct kobj_t * kobj;
	if(offset < 0)//不支持负数
	  return -1;
	if(n->v_type != VNT_REG)
	  return -2;
	kobj = n->v_data;
	if(kobj && kobj->read)
	  return kobj->read(kobj, offset, buf, len);
	return -3;
}

/********************************************************************
*                      功能函数
*功能描述：写入
*输入参数：n: kobj节点信息
*		  off: 偏移地址
*		  buf: 写入缓冲区
*		  len: 写入长度
*返回值：写入长度
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int64_t sys_write(struct vfs_node_t * n, int64_t offset, void * buf, int64_t len)
{
	struct kobj_t * kobj;
	if(offset < 0)//不支持负数
	  return -1;
	if(n->v_type != VNT_REG)
		return -2;
	kobj = n->v_data;
	if(kobj && kobj->write)
	  return kobj->write(kobj, offset, buf, len);
	return -3;
}

/********************************************************************
*                      功能函数
*功能描述：控制
*输入参数：n: kobj节点信息
*		  cmd: 命令
*		  buf: 附带的参数，建议写NULL
*返回值：OK
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int64_t sys_ioctl(struct vfs_node_t * n, uint64_t cmd, void *buf)
{
	struct kobj_t * kobj;

	if(n->v_type != VNT_REG)
	  return -1;
	kobj = n->v_data;
	if(kobj && kobj->ioctl)
	  return kobj->ioctl(kobj, cmd, buf);
	return -1;
}

/********************************************************************
*                      功能函数
*功能描述：读取目录
*输入参数：dn: kobj节点信息
*		  off: 偏移地址
*		  d: 返回目录信息
*返回值：写入长度
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int sys_readdir(struct vfs_node_t * dn, int64_t off, struct vfs_dirent_t * d)
{
	struct kobj_t * kobj, * obj;
	struct list_head * pos;
	int i;

	kobj = dn->v_data;
	if(list_empty(&kobj->children))
	 return -1;

	pos = (&kobj->children)->next;
	for(i = 0; i != off; i++)
	 {
		pos = pos->next;
		if(pos == (&kobj->children))
		  return -1;
	 }

	obj = list_entry(pos, struct kobj_t, entry);
	if(obj->type == KOBJ_TYPE_DIR)
	  d->d_type = VDT_DIR;
	else
	  d->d_type = VDT_REG;
	strncpy(d->d_name, obj->name, sizeof(d->d_name));
	d->d_off = off;
	d->d_reclen = 1;

	return 0;
}
/********************************************************************
*                      功能函数
*功能描述：向上查找
*输入参数：dn: kobj节点信息
*		  name: 偏移地址
*		  n: 返回目录信息
*返回值：写入长度
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.10.03     1.0        刘杨
**********************************************************************/
static int sys_lookup(struct vfs_node_t * dn, const char * name, struct vfs_node_t * n)
{
	struct kobj_t * kobj, * obj;

	if(*name == '\0')
	 return -1;

	kobj = dn->v_data;
	obj = kobj_search(kobj, name);
	if(!obj)
	 return -1;

	n->v_mode = 0;
	n->v_size = obj->size;//寄存器寻址大小
	n->v_data = (void *)obj;

	if(obj->type == KOBJ_TYPE_DIR)
	 {
		n->v_type = VNT_DIR;
		n->v_mode |= S_IFDIR;
		n->v_mode |= S_IRWXU;
	 }
	else
	 {
		n->v_type = VNT_REG;
		n->v_mode |= S_IFREG;
		if(obj->read)
		  n->v_mode |= S_IRUSR;
		if(obj->write)
		  n->v_mode |= S_IWUSR;
		if(obj->ioctl)
		  n->v_mode |= S_IXUSR;
	 }
	return 0;
}

static struct filesystem_t sys = {
	.name		= "sys",

	.mount		= sys_mount,
	.unmount	= sys_unmount,
	.read		= sys_read,
	.write		= sys_write,
	.ioctl		= sys_ioctl,
	.readdir	= sys_readdir,
	.lookup		= sys_lookup,
};

void filesystem_sys_init(void)
{
	register_filesystem(&sys);
}

void filesystem_sys_exit(void)
{
	unregister_filesystem(&sys);
}
