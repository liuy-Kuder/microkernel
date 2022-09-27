#include "microkernel.h"
#include "sysfs.h"
#include "vfs.h"

static int sys_mount(struct vfs_mount_t * m, const char * dev)
{
	if(dev)
		return -1;

	m->m_flags |= MOUNT_RO;
	m->m_root->v_data = (void *)kobj_get_root();
	m->m_data = NULL;
	return 0;
}

static int sys_unmount(struct vfs_mount_t * m)
{
	m->m_data = NULL;
	return 0;
}

static int sys_msync(struct vfs_mount_t * m)
{
	return 0;
}

static int sys_vget(struct vfs_mount_t * m, struct vfs_node_t * n)
{
	return 0;
}

static int sys_vput(struct vfs_mount_t * m, struct vfs_node_t * n)
{
	return 0;
}

static uint64_t sys_read(struct vfs_node_t * n, int64_t off, void * buf, uint64_t len)
{
	struct kobj_t * kobj;

	if(n->v_type != VNT_REG)
		return 0;

	kobj = n->v_data;
	if(off == 0)
	{
		if(kobj && kobj->read)
			return kobj->read(kobj, buf, len);
	}
	return 0;
}

static uint64_t sys_write(struct vfs_node_t * n, int64_t off, void * buf, uint64_t len)
{
	struct kobj_t * kobj;

	if(n->v_type != VNT_REG)
		return 0;

	kobj = n->v_data;
	if(off == 0)
	{
		if(kobj && kobj->write)
			return kobj->write(kobj, buf, len);
	}
	return 0;
}

static uint64_t sys_ioctl(struct vfs_node_t * n, uint16_t cmd, void * buf)
{
	struct kobj_t * kobj;

	if(n->v_type != VNT_REG)
		return 0;

	kobj = n->v_data;

	if(kobj && kobj->ioctl)
		return kobj->ioctl(kobj, cmd, buf);
		
	return 0;
}

static int sys_truncate(struct vfs_node_t * n, int64_t off)
{
	return -1;
}

static int sys_sync(struct vfs_node_t * n)
{
	return 0;
}

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
//
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
	n->v_size = 0;
	n->v_data = (void *)obj;

	if(obj->type == KOBJ_TYPE_DIR)
	{
		n->v_type = VNT_DIR;
		n->v_mode |= S_IFDIR;
		n->v_mode |= S_IRWXU | S_IRWXG | S_IRWXO;
	}
	else
	{
		n->v_type = VNT_REG;
		n->v_mode |= S_IFREG;
		if(obj->read)
			n->v_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
		if(obj->write)
			n->v_mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
	}
	return 0;
}

static int sys_create(struct vfs_node_t * dn, const char * filename, uint32_t mode)
{
	return -1;
}

static int sys_remove(struct vfs_node_t * dn, struct vfs_node_t * n, const char *name)
{
	return -1;
}

static int sys_rename(struct vfs_node_t * sn, const char * sname, struct vfs_node_t * n, struct vfs_node_t * dn, const char * dname)
{
	return -1;
}

static int sys_mkdir(struct vfs_node_t * dn, const char * name, uint32_t mode)
{
	return -1;
}

static int sys_rmdir(struct vfs_node_t * dn, struct vfs_node_t * n, const char *name)
{
	return -1;
}

static int sys_chmod(struct vfs_node_t * n, uint32_t mode)
{
	return -1;
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
#if 1
	.msync		= sys_msync,
	.vget		= sys_vget,
	.vput		= sys_vput,

	.truncate	= sys_truncate,
	.sync		= sys_sync,
	.create		= sys_create,
	.remove		= sys_remove,
	.rename		= sys_rename,
	.mkdir		= sys_mkdir,
	.rmdir		= sys_rmdir,
	.chmod		= sys_chmod,
#endif
};

void filesystem_sys_init(void)
{
	register_filesystem(&sys);
}

void filesystem_sys_exit(void)
{
	unregister_filesystem(&sys);
}
