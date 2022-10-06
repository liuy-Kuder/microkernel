/********************************************************************
*
*文件名称：vfs.c
*内容摘要：文件系统调用驱动、总线层的功能，做集成管理
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.09.26
*其他说明: 仅支持驱动、总线的调用！！
*
**********************************************************************/
#include "microkernel.h"
#include "sysfs.h"
#include "vfs.h"
#include "rtos.h"

struct list_head __filesystem_list = {
	.next = &__filesystem_list,
	.prev = &__filesystem_list,
};

static struct kobj_t * search_class_filesystem_kobj(void)
{
	//struct kobj_t * kclass = kobj_search_directory_with_create(kobj_get_root(), "class");
	return kobj_search_directory_with_create(kobj_get_root(), "filesystem");
}

struct filesystem_t * search_filesystem(const char * name)
{
	struct filesystem_t * pos, * n;

	if(!name)
		return NULL;

	list_for_each_entry_safe(pos, n,struct filesystem_t, &__filesystem_list, list)
	{
		if(strcmp(pos->name, name) == 0)
			return pos;
	}
	return NULL;
}

uint8_t register_filesystem(struct filesystem_t * fs)
{
	if(!fs || !fs->name)
		return FALSE;

	if(search_filesystem(fs->name))
		return FALSE;

	fs->kobj = kobj_alloc_directory(fs->name); 
	kobj_add(search_class_filesystem_kobj(), fs->kobj);
	spin_lock_irq();
	list_add_tail(&fs->list, &__filesystem_list);
	spin_unlock_irq();

	return TRUE;
}

uint8_t unregister_filesystem(struct filesystem_t * fs)
{
	if(!fs || !fs->name)
		return FALSE;

	spin_lock_irq();
	list_del(&fs->list);
	spin_unlock_irq();
	kobj_remove(search_class_filesystem_kobj(), fs->kobj);
	kobj_remove_self(fs->kobj);

	return TRUE;
}

struct vfs_file_t {
#ifdef USE_FREERTOS
	SemaphoreHandle_t f_lock;
//#elif USER_THREADX
#endif
	struct vfs_node_t * f_node;
	int64_t f_offset;
	uint32_t f_flags;
};

#ifdef USE_FREERTOS
	SemaphoreHandle_t mnt_list_lock;
	SemaphoreHandle_t fd_file_lock;
	SemaphoreHandle_t node_list_lock[VFS_NODE_HASH_SIZE];
#endif

static struct list_head mnt_list;
static struct vfs_file_t fd_file[VFS_MAX_FD];
struct list_head node_list[VFS_NODE_HASH_SIZE];

//数量匹配
static int count_match(const char * path, char * mount_root)
{
	int len = 0;

	while(*path && *mount_root)
	{
		if(*path != *mount_root)
			break;
		path++;
		mount_root++;
		len++;
	}

	if(*mount_root != '\0')
		return 0;

	if((len == 1) && (*(path - 1) == '/'))
		return 1;

	if((*path == '\0') || (*path == '/'))
		return len;

	return 0;
}

//寻找根节点
//报错返回 < 0 
static int vfs_findroot(const char * path, struct vfs_mount_t ** mp, char ** root)
{
	struct vfs_mount_t * pos, * m = NULL;
	int len, max_len = 0;

	if(!path || !mp || !root)
		return -1;

	mutex_lock(mnt_list_lock);
	list_for_each_entry(pos,struct vfs_mount_t, &mnt_list, m_link)
	{
		len = count_match(path, pos->m_path);
		if(len > max_len)
		{
			max_len = len;
			m = pos;
		}
	}
	mutex_unlock(mnt_list_lock);

	if(!m)
		return -1;

	*root = (char *)(path + max_len);
	while(**root == '/')
	{
		(*root)++;
	}
	*mp = m;

	return 0;
}
//分配fd描述符
static int vfs_fd_alloc(void)
{
	int i, fd = -1;

	mutex_lock(fd_file_lock);
	for(i = 3; i < VFS_MAX_FD; i++)
	{
		if(fd_file[i].f_node == NULL)
		{
			fd = i;
			break;
		}
	}
	mutex_unlock(fd_file_lock);

	return fd;
}

static void vfs_fd_free(int fd)
{
	if((fd >= 3) && (fd < VFS_MAX_FD))
	{
		mutex_lock(fd_file_lock);
		if(fd_file[fd].f_node)
		{
			mutex_lock(fd_file[fd].f_lock);
			fd_file[fd].f_node = NULL;
			fd_file[fd].f_offset = 0;
			fd_file[fd].f_flags = 0;
			mutex_unlock(fd_file[fd].f_lock);
		}
		mutex_unlock(fd_file_lock);
	}
}

static struct vfs_file_t * vfs_fd_to_file(int fd)
{
	return ((fd >= 0) && (fd < VFS_MAX_FD)) ? &fd_file[fd] : NULL;
}

static uint32_t vfs_node_hash(struct vfs_mount_t * m, const char * path)
{
	uint32_t val = 0;

	if(path)
	{
		while(*path)
			val = ((val << 5) + val) + *path++;
	}
	return (val ^ (uint32_t)((unsigned long)m)) & (VFS_NODE_HASH_SIZE - 1);
}

static struct vfs_node_t * vfs_node_get(struct vfs_mount_t * m, const char * path)
{
	struct vfs_node_t * n;
	uint32_t hash = vfs_node_hash(m, path);

	//if(!(n = calloc(1, sizeof(struct vfs_node_t))))
	if(!(n = MK_MALLOC(sizeof(struct vfs_node_t))))
		return NULL;

	init_list_head(&n->v_link);
	mutex_init(n->v_lock);
	n->v_mount = m;
	
    strncpy(n->v_path, path, sizeof(n->v_path));
/*	if(strlcpy(n->v_path, path, sizeof(n->v_path)) >= sizeof(n->v_path))
	{
		free(n);
		return NULL;
	}
*/
	mutex_lock(node_list_lock[hash]);
	list_add(&n->v_link, &node_list[hash]);
	mutex_unlock(node_list_lock[hash]);
	return n;
}

static struct vfs_node_t * vfs_node_lookup(struct vfs_mount_t * m, const char * path)
{
	struct vfs_node_t * n;
	uint32_t hash = vfs_node_hash(m, path);
	int found = 0;

	mutex_lock(node_list_lock[hash]);
	list_for_each_entry(n,struct vfs_node_t, &node_list[hash], v_link)
	{
		if((n->v_mount == m) && (!strncmp(n->v_path, path, VFS_MAX_PATH)))
		{
			found = 1;
			break;
		}
	}
	mutex_unlock(node_list_lock[hash]);

	if(!found)
		return NULL;

	return n;
}

static void vfs_node_put(struct vfs_node_t * n)
{
	uint32_t hash;
	hash = vfs_node_hash(n->v_mount, n->v_path);
	vfs_node_hash(n->v_mount, n->v_path);
	mutex_lock(node_list_lock[hash]);
	list_del(&n->v_link);
	mutex_unlock(node_list_lock[hash]);
	MK_FREE(n);
}

static int vfs_node_stat(struct vfs_node_t * n, struct vfs_stat_t * st)
{
	uint32_t mode;

	memset(st, 0, sizeof(struct vfs_stat_t));

	st->st_ino = (uint64_t)((unsigned long)n);
	mutex_lock(n->v_lock);
	st->st_size = n->v_size;
	mode = n->v_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
	mutex_unlock(n->v_lock);
	switch(n->v_type)
	{
	case VNT_REG:
		mode |= S_IFREG;
		break;
	case VNT_DIR:
		mode |= S_IFDIR;
		break;
	case VNT_BLK:
		mode |= S_IFBLK;
		break;
	case VNT_CHR:
		mode |= S_IFCHR;
		break;
	case VNT_LNK:
		mode |= S_IFLNK;
		break;
	case VNT_SOCK:
		mode |= S_IFSOCK;
		break;
	case VNT_FIFO:
		mode |= S_IFIFO;
		break;
	default:
		return -1;
	};
	st->st_mode = mode;

	if(n->v_type == VNT_CHR || n->v_type == VNT_BLK)
		st->st_dev = (uint64_t)((unsigned long)n->v_data);
	st->st_uid = 0;
	st->st_gid = 0;

	return 0;
}

static int vfs_node_access(struct vfs_node_t * n, uint32_t mode)
{
	uint32_t m;

	mutex_lock(n->v_lock);
	m = n->v_mode;
	mutex_unlock(n->v_lock);

	if((mode & R_OK) && !(m & (S_IRUSR | S_IRGRP | S_IROTH)))
		return -1;

	if(mode & W_OK)
	{
		if(n->v_mount->m_flags & MOUNT_RO)
			return -1;

		if(!(m & (S_IWUSR | S_IWGRP | S_IWOTH)))
			return -1;
	}

	if((mode & X_OK) && !(m & (S_IXUSR | S_IXGRP | S_IXOTH)))
		return -1;

	return 0;
}

static void vfs_node_release(struct vfs_node_t * n)
{
	struct vfs_mount_t * m;
	char path[VFS_MAX_PATH];
	char * p;

	if(!n)
		return;

	m = n->v_mount;

	if(m->m_root == n)
	{
		vfs_node_put(n);
		return;
	}
    strncpy(path, n->v_path, sizeof(path));
/*	if(strlcpy(path, n->v_path, sizeof(path)) >= sizeof(path))
		return;
*/
	vfs_node_put(n);

	while(1)
	{
		p = strrchr(path, '/');
		if(!p)
			break;
		*p = '\0';

		if(path[0] == '\0')
			break;

		n = vfs_node_lookup(m, path);
		if(!n)
			continue;

		vfs_node_put(n);
	}
	vfs_node_put(m->m_root);
}

//获取节点
static int vfs_node_acquire(const char * path, struct vfs_node_t ** np)
{
	struct vfs_mount_t * m;
	struct vfs_node_t * dn, * n;
	char node[VFS_MAX_PATH];
	char * p;
	int err, i, j;

	if(vfs_findroot(path, &m, &p))
		return -1;

	if(!m->m_root)
		return -1;

	dn = n = m->m_root;

	i = 0;
	while(*p != '\0')//不为空
	{
		while(*p == '/')
			p++;

		if(*p == '\0')
			break;

		node[i] = '/';
		i++;
		j = i;
		while(*p != '\0' && *p != '/')
		{
			node[i] = *p;
			p++;
			i++;
		}
		node[i] = '\0';

		n = vfs_node_lookup(m, node);
		if(n == NULL)
		{
			n = vfs_node_get(m, node);
			if(n == NULL)
			{
				vfs_node_put(dn);
				return -1;
			}

			mutex_lock(n->v_lock);
			mutex_lock(dn->v_lock);
			err = dn->v_mount->m_fs->lookup(dn, &node[j], n);
			mutex_unlock(dn->v_lock);
			mutex_unlock(n->v_lock);
			if(err || (*p == '/' && n->v_type != VNT_DIR))
			{
				vfs_node_release(n);
				return err;
			}
		}
		dn = n;
	}
	*np = n;

	return 0;
}

void vfs_force_unmount(struct vfs_mount_t * m)
{
	struct vfs_mount_t * tm;
	struct vfs_node_t * n;
	int found;
	int i;

	while(1)
	{
		found = 0;
		list_for_each_entry(tm, struct vfs_mount_t, &mnt_list, m_link)
		{
			if(tm->m_covered && tm->m_covered->v_mount == m)
			{
				found = 1;
				break;
			}
		}
		if(!found)
			break;
		vfs_force_unmount(tm);
	}
	list_del(&m->m_link);

	mutex_lock(fd_file_lock);
	for(i = 0; i < VFS_MAX_FD; i++)
	{
		if(fd_file[i].f_node && (fd_file[i].f_node->v_mount == m))
		{
			mutex_lock(fd_file[i].f_lock);
			fd_file[i].f_node = NULL;
			fd_file[i].f_offset = 0;
			fd_file[i].f_flags = 0;
			mutex_unlock(fd_file[i].f_lock);
		}
	}
	mutex_unlock(fd_file_lock);

	for(i = 0; i < VFS_NODE_HASH_SIZE; i++)
	{
		mutex_lock(node_list_lock[i]);
		while(1)
		{
			found = 0;
			list_for_each_entry(n, struct vfs_node_t, &node_list[i], v_link)
			{
				if(n->v_mount == m)
				{
					found = 1;
					break;
				}
			}
			if(!found)
				break;

			list_del(&n->v_link);
			MK_FREE(n);
		}
		mutex_unlock(node_list_lock[i]);
	}

	mutex_lock(m->m_lock);
	m->m_fs->unmount(m);
	mutex_unlock(m->m_lock);

	if(m->m_covered)
		vfs_node_release(m->m_covered);
	MK_FREE(m);
}

int vfs_mount(const char * dev, const char * dir, const char * fsname, uint32_t flags)
{
	//struct block_t * bdev;
	struct filesystem_t * fs;
	struct vfs_mount_t * m, * tm;
	struct vfs_node_t * n, * n_covered;
	int err;

	if(!dir || *dir == '\0')
	 {
		return -1;
	 }
		

	if(!(fs = search_filesystem(fsname)))//必须先注册fsname
	 {
		return -1;
	 }

	//if(!(m = calloc(1, sizeof(struct vfs_mount_t))))
	if(!(m = MK_MALLOC(sizeof(struct vfs_mount_t))))
	 {
		return -1;
	 }

	init_list_head(&m->m_link);
	mutex_init(m->m_lock);
	m->m_fs = fs;
	m->m_flags = flags & MOUNT_MASK;

	if(sizeof(dir) < VFS_MAX_PATH)
      {
		strncpy(m->m_path, dir, sizeof(m->m_path));
	  }
	else
	  {
		MK_FREE(m);
		return -1;
	  }
	//m->m_dev = bdev;
	if(*dir == '/' && *(dir + 1) == '\0')
	{
		n_covered = NULL;
	}
	else
	{
		if(vfs_node_acquire(dir, &n_covered) != 0)
		{
			MK_FREE(m);
			return -1;
		}
		if(n_covered->v_type != VNT_DIR)
		{
			vfs_node_release(n_covered);
			MK_FREE(m);
			return -1;
		}
	}
	m->m_covered = n_covered;

	if(!(n = vfs_node_get(m, "/")))
	{
		if(m->m_covered)
			vfs_node_release(m->m_covered);
		MK_FREE(m);
		return -1;
	}
	n->v_type = VNT_DIR;
	n->v_flags = VNF_ROOT;
	n->v_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
	m->m_root = n;

	mutex_lock(m->m_lock);
	err = m->m_fs->mount(m, dev);
	mutex_unlock(m->m_lock);
	if(err != 0)
	 {
		vfs_node_release(m->m_root);
		if(m->m_covered)
		  vfs_node_release(m->m_covered);
		MK_FREE(m);
		return err;
	 }

	if(m->m_flags & MOUNT_RO)
		m->m_root->v_mode &= ~(S_IWUSR|S_IWGRP|S_IWOTH);

	mutex_lock(mnt_list_lock);
	list_for_each_entry(tm, struct vfs_mount_t, &mnt_list, m_link)
	{
        if(!strcmp(tm->m_path, dir))
		{
			mutex_unlock(mnt_list_lock);
			mutex_lock(m->m_lock);
			m->m_fs->unmount(m);
			mutex_unlock(m->m_lock);
			vfs_node_release(m->m_root);
			if(m->m_covered)
			  vfs_node_release(m->m_covered);
			MK_FREE(m);
			return -1;
		}
    }
	list_add(&m->m_link, &mnt_list);
	mutex_unlock(mnt_list_lock);

	return 0;
}

//文件系统卸载
int vfs_unmount(const char * path)
{
	struct vfs_mount_t * m;
	int found;

	mutex_lock(mnt_list_lock);
	found = 0;
	list_for_each_entry(m,struct vfs_mount_t, &mnt_list, m_link)
	{
		if(!strcmp(path, m->m_path))
		{
			found = 1;
			break;
		}
	}
	if(!found)
	 {
		mutex_unlock(mnt_list_lock);
		return -1;
	 }

	list_del(&m->m_link);
	mutex_unlock(mnt_list_lock);

	mutex_lock(m->m_lock);
	m->m_fs->unmount(m);
	mutex_unlock(m->m_lock);

	vfs_node_release(m->m_root);
	if(m->m_covered)
	  vfs_node_release(m->m_covered);
	MK_FREE(m);

	return 0;
}

struct vfs_mount_t * vfs_mount_get(int index)
{
	struct vfs_mount_t * m = NULL;
	int found = 0;

	if(index < 0)
		return NULL;

	mutex_lock(mnt_list_lock);
	list_for_each_entry(m,struct vfs_mount_t, &mnt_list, m_link)
	{
		if(!index)
		{
			found = 1;
			break;
		}
		index--;
	}
	mutex_unlock(mnt_list_lock);

	if(!found)
		return NULL;
	return m;
}

//VFS挂载数量
int vfs_mount_count(void)
{
	struct vfs_mount_t * m;
	uint8_t ret = 0;

	mutex_lock(mnt_list_lock);
	list_for_each_entry(m, struct vfs_mount_t, &mnt_list, m_link)
	{
		ret++;
	}
	mutex_unlock(mnt_list_lock);

	return ret;
}

static int vfs_lookup_dir(const char * path, struct vfs_node_t ** np, char ** name)
{
	struct vfs_node_t * n;
	char buf[VFS_MAX_PATH];
	char * file, * dir;
	int err;
    strncpy(buf, path, sizeof(buf));
	/*if(strncpy(buf, path, sizeof(buf)) >= sizeof(buf))
		return -1;
*/
	file = strrchr(buf, '/');
	if(!file)
		return -1;

	if(!buf[0])
		return -1;
	if(file == buf)
	{
		dir = "/";
	}
	else
	{
		*file = '\0';
		dir = buf;
	}

	if((err = vfs_node_acquire(dir, &n)))
		return err;
	if(n->v_type != VNT_DIR)
	{
		vfs_node_release(n);
		return -1;
	}
	*np = n;

	*name = strrchr(path, '/');
	if(*name == NULL)
		return -1;
	*name += 1;

	return 0;
}

//打开一个节点
int vfs_open(const char * path, uint32_t flags)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int err, fd;

	if(!path || !(flags & O_ACCMODE))//不支持读写双工
		return -1;

    if((err = vfs_node_acquire(path, &n)))
    {
        return err;
    }

    if(flags & O_WRONLY)
    {
        if((err = vfs_node_access(n, W_OK)))
        {
            vfs_node_release(n);
            return err;
        }
        if(n->v_type == VNT_DIR)
        {
            vfs_node_release(n);
            return -1;
        }
    }
	
	fd = vfs_fd_alloc();
	if(fd < 0)
	{
		vfs_node_release(n);
		return -1;
	}
	f = vfs_fd_to_file(fd);

	mutex_lock(f->f_lock);
	f->f_node = n;
	f->f_offset = 0;
	f->f_flags = flags;
	mutex_unlock(f->f_lock);

	return fd;
}

//关闭文件
int vfs_close(int fd)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}

	vfs_node_release(n);
	mutex_unlock(f->f_lock);

	vfs_fd_free(fd);
	return 0;
}
//文件节点的读取
int32_t vfs_read(int fd, void * buf, uint32_t len)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int32_t ret;

	if(!buf || !len)
	 {
		ret = -1;
		return ret;
	 }

	f = vfs_fd_to_file(fd);
	if(!f)
	 {
		ret = -1;
		return ret;
	 }

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	}
	if(n->v_type != VNT_REG)
	{
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	}

	if(!(f->f_flags & O_RDONLY))
	{
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	}

	mutex_lock(n->v_lock);
	//ret = n->v_mount->m_fs->read(n, f->f_offset, buf, len);
	ret = n->v_mount->m_fs->read(n, 0, buf, len);
	mutex_unlock(n->v_lock);

	//f->f_offset += ret;
	mutex_unlock(f->f_lock);

	return ret;
}

//文件节点的写入
int32_t vfs_write(int fd, void * buf, uint32_t len)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int32_t ret = 0;

	if(!buf || !len)
	 {
		ret = -1;
		return ret;
	 }

	f = vfs_fd_to_file(fd);
	if(!f)
	 {
		ret = -1;
		return ret;
	 }

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	 {
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	 }
	if(n->v_type != VNT_REG)
	 {
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	 }

	if(!(f->f_flags & O_WRONLY))
	 {
		mutex_unlock(f->f_lock);
		ret = -1;
		return ret;
	 }

	mutex_lock(n->v_lock);
	//ret = n->v_mount->m_fs->write(n, f->f_offset, buf, len);
	ret = n->v_mount->m_fs->write(n, 0, buf, len);
	mutex_unlock(n->v_lock);

	//f->f_offset += ret;
	mutex_unlock(f->f_lock);
	return ret;
}

//文件节点的读取
int16_t vfs_ioctl(int fd, uint64_t cmd, void * buf)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int16_t err = 0;
	f = vfs_fd_to_file(fd);
	if(!f)
	 {
		err = -1;
		return err;
	 }
	
	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		err = -1;
		return err;
	}
	if(n->v_type != VNT_REG)
	{
		mutex_unlock(f->f_lock);
		err = -1;
		return err;
	}

	if(!(f->f_flags & O_CTLONLY))
	{
		mutex_unlock(f->f_lock);
		err = -1;
		return err;
	}

	mutex_lock(n->v_lock);
	err = n->v_mount->m_fs->ioctl(n, cmd, buf);
	mutex_unlock(n->v_lock);
	mutex_unlock(f->f_lock);

	return err;
}

int64_t vfs_lseek(int fd, int64_t off, int whence)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int64_t ret;

	f = vfs_fd_to_file(fd);
	if(!f)
		return 0;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return 0;
	}

	mutex_lock(n->v_lock);
	switch(whence)
	{
	case VFS_SEEK_SET:
		if(off < 0)
			off = 0;
		else if(off > (signed long long)n->v_size)
			off = n->v_size;
		break;

	case VFS_SEEK_CUR:
		if((f->f_offset + off) > (signed long long)n->v_size)
			off = n->v_size;
		else if((f->f_offset + off) < 0)
			off = 0;
		else
			off = f->f_offset + off;
		break;

	case VFS_SEEK_END:
		if(off > 0)
			off = n->v_size;
		else if((n->v_size + off) < 0)
			off = 0;
		else
			off = n->v_size + off;
		break;

	default:
		mutex_unlock(n->v_lock);
		ret = f->f_offset;
		mutex_unlock(f->f_lock);
		return ret;
	}

	if(off <= n->v_size)
		f->f_offset = off;
	mutex_unlock(n->v_lock);
	ret = f->f_offset;
	mutex_unlock(f->f_lock);

	return ret;
}

int vfs_fstat(int fd, struct vfs_stat_t * st)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int err;

	if(!st)
		return -1;

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	err = vfs_node_stat(n, st);
	mutex_unlock(f->f_lock);

	return err;
}

//打开一个目录
int vfs_opendir(const char * name)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int fd;

	if(!name)
		return -1;

	if((fd = vfs_open(name, O_RDONLY)) < 0)
		return fd;//不是目录报错返回

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}

	if(n->v_type != VNT_DIR)
	{
		mutex_unlock(f->f_lock);
		vfs_close(fd);
		return -1;
	}
	mutex_unlock(f->f_lock);

	return fd;
}
//关闭一个目录
int vfs_closedir(int fd)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}

	if(n->v_type != VNT_DIR)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	mutex_unlock(f->f_lock);

	return vfs_close(fd);
}

//读取一个目录
int vfs_readdir(int fd, struct vfs_dirent_t * dir)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;
	int err;

	if(!dir)
		return -1;

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	if(n->v_type != VNT_DIR)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	mutex_lock(n->v_lock);
	err = n->v_mount->m_fs->readdir(n, f->f_offset, dir);
	mutex_unlock(n->v_lock);
	if(!err)
		f->f_offset += dir->d_reclen;
	mutex_unlock(f->f_lock);

	return err;
}

int vfs_rewinddir(int fd)
{
	struct vfs_node_t * n;
	struct vfs_file_t * f;

	f = vfs_fd_to_file(fd);
	if(!f)
		return -1;

	mutex_lock(f->f_lock);
	n = f->f_node;
	if(!n)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	if(n->v_type != VNT_DIR)
	{
		mutex_unlock(f->f_lock);
		return -1;
	}
	f->f_offset = 0;
	mutex_unlock(f->f_lock);

	return 0;
}

static int vfs_check_dir_empty(const char * path)
{
	struct vfs_dirent_t dir;
	int err, fd, count;

	if((fd = vfs_opendir(path)) < 0)
		return fd;

	count = 0;
	do
	{
		err = vfs_readdir(fd, &dir);
		if(err)
			break;
		if((strcmp(dir.d_name, ".") != 0) && (strcmp(dir.d_name, "..") != 0))
			count++;
		if(count)
			break;
	} while(1);

	vfs_closedir(fd);
	if(count)
		return -1;

	return 0;
}

int vfs_unlink(const char * path)
{
	struct vfs_node_t * n, * dn;
	char * name;
	int err;

	if(!path)
		return -1;

	if((err = vfs_node_acquire(path, &n)))
		return err;

	if(n->v_type == VNT_DIR)
	{
		vfs_node_release(n);
		return -1;
	}

    if(n->v_flags == VNF_ROOT) 
	{
		vfs_node_release(n);
		return -1;
	}

	if((err = vfs_node_access(n, W_OK)))
	{
		vfs_node_release(n);
		return err;
	}

	if((err = vfs_lookup_dir(path, &dn, &name)))
	{
		vfs_node_release(n);
		return err;
	}

	vfs_node_release(dn);
	vfs_node_release(n);

	return err;
}

int vfs_access(const char * path, uint32_t mode)
{
	struct vfs_node_t * n;
	int err;

	if(!path)
		return -1;

	if((err = vfs_node_acquire(path, &n)))
		return err;

	err = vfs_node_access(n, mode);
	vfs_node_release(n);

	return err;
}

int vfs_stat(const char * path, struct vfs_stat_t * st)
{
	struct vfs_node_t * n;
	int err;

	if(!path || !st)
		return -1;

	if((err = vfs_node_acquire(path, &n)))
		return err;
	err = vfs_node_stat(n, st);
	vfs_node_release(n);

	return err;
}

void do_init_vfs(void)
{
	int i;

	init_list_head(&mnt_list);
	mutex_init(mnt_list_lock);

	for(i = 0; i < VFS_MAX_FD; i++)
	{
		mutex_init(fd_file[i].f_lock);
		fd_file[i].f_node = NULL;
		fd_file[i].f_offset = 0;
		fd_file[i].f_flags = 0;
	}
	mutex_init(fd_file_lock);

	for(i = 0; i < VFS_NODE_HASH_SIZE; i++)
	{
		init_list_head(&node_list[i]);
		mutex_init(node_list_lock[i]);
	}
}

