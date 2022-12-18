#include "microkernel.h"
#include "mk_log.h"

/********************************************************************
*                      功能函数
*功能描述：MK内核初始化
*		  1、自动加载MircoKernel初始化
*		  2、总线资源初始化
*		  3、驱动资源初始化
*		  4、挂载文件系统
*输入参数：无
*返回值：无
*其他说明：无
*修改日期       版本      修改人        修改内容
*---------------------------------------------------------------------
*2022.9.12     1.0        刘杨
**********************************************************************/
int mk_init(void)
{
	int err = 0;
	driver_pure_init();//初始化总线列表
	device_pure_init();//初始化驱动列表
	do_init_vfs();//初始化文件列表
	filesystem_sys_init();//注册系统文件
	err = vfs_mount(NULL, "/", "sys", MOUNT_RW);//挂载文件
    MK_LOG_INFO("mircokernel v%d.%d.%d\n" LVGL_VERSION_INFO,LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR,LVGL_VERSION_PATCH);
	return err;
}
