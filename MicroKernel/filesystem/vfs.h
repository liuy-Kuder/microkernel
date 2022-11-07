#ifndef __VFS_H__
#define __VFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "microkernel.h"
#include "sysfs.h"
#include <stdint.h>
#include "rtos.h"

//VFS系统
#define VFS_MAX_PATH		(1024)//绝对路径的最大字符
#define	VFS_MAX_NAME		(64)
#define VFS_MAX_FD			(64)
#define VFS_NODE_HASH_SIZE	(64)

#define O_RDONLY			(1 << 0)//只读
#define O_WRONLY			(1 << 1)//只写
#define O_CTLONLY			(1 << 2)//控制
#define O_RDWR				(O_RDONLY | O_WRONLY | O_CTLONLY)//读写
#define O_ACCMODE			(O_RDWR)//全模式

#define S_IXUSR				(1 << 0) //所有者拥有执行权限
#define S_IWUSR				(1 << 1) //所有者拥有写权限
#define S_IRUSR				(1 << 2) //所有者拥有读权限
#define S_IRWXU				(S_IRUSR | S_IWUSR | S_IXUSR)

#define	S_IFDIR				(1 << 3)
#define	S_IFCHR				(1 << 4)
#define	S_IFBLK				(1 << 5)
#define	S_IFREG				(1 << 6)
#define	S_IFLNK				(1 << 7)
#define	S_IFIFO				(1 << 8)
#define	S_IFSOCK			(1 << 9)
#define	S_IFMT				(S_IFDIR | S_IFCHR | S_IFBLK | S_IFREG | S_IFLNK | S_IFIFO | S_IFSOCK)

#define S_ISDIR(m)			((m) & S_IFDIR )
#define S_ISCHR(m)			((m) & S_IFCHR )
#define S_ISBLK(m)			((m) & S_IFBLK )
#define S_ISREG(m)			((m) & S_IFREG )
#define S_ISLNK(m)			((m) & S_IFLNK )
#define S_ISFIFO(m)			((m) & S_IFIFO )
#define S_ISSOCK(m)			((m) & S_IFSOCK )

#define	C_OK				(1 << 3) //是否具有控制权限
#define	R_OK				(1 << 2) //是否具有读权限
#define	W_OK				(1 << 1) //是否具有可写权限
#define	X_OK				(1 << 0) //是否具有可执行权限

#define VFS_SEEK_SET		(0)
#define VFS_SEEK_CUR		(1)
#define VFS_SEEK_END		(2)

struct vfs_stat_t;
struct vfs_dirent_t;
struct vfs_node_t;
struct vfs_mount_t;
struct filesystem_t;

struct vfs_stat_t {
	uint64_t st_ino;
	uint32_t st_mode;
	uint64_t st_dev;
};

enum vfs_dirent_type_t {
	VDT_UNK,
	VDT_DIR,
	VDT_CHR,
	VDT_BLK,
	VDT_REG,
	VDT_LNK,
	VDT_FIFO,
	VDT_SOCK,
};

struct vfs_dirent_t {
	uint64_t d_off;
	uint32_t d_reclen;
	enum vfs_dirent_type_t d_type;
	char d_name[VFS_MAX_NAME];
};

enum vfs_node_flag_t {
	VNF_NONE,
	VNF_ROOT,
};

enum vfs_node_type_t {
	VNT_UNK,
	VNT_DIR,
	VNT_CHR,
	VNT_BLK,
	VNT_REG,
	VNT_LNK,
	VNT_FIFO,
	VNT_SOCK,
};

struct vfs_node_t {
	struct list_head v_link;
	struct vfs_mount_t * v_mount;

	char v_path[VFS_MAX_PATH];
	enum vfs_node_flag_t v_flags;
	enum vfs_node_type_t v_type;
#ifdef USE_FREERTOS
	SemaphoreHandle_t v_lock;
#endif
#ifdef USE_THREADX
	TX_MUTEX v_lock;
#endif
	uint16_t v_mode;
	size_t v_size;
	void * v_data;
};

enum {
	MOUNT_RW	= (0x0 << 0),
	MOUNT_RO	= (0x1 << 0),
	MOUNT_MASK	= (0x1 << 0),
};

struct vfs_mount_t {
	struct list_head m_link;
	struct filesystem_t * m_fs;
	char m_path[VFS_MAX_PATH];
	uint32_t m_flags;
	struct vfs_node_t * m_root;
	struct vfs_node_t * m_covered;
#ifdef USE_FREERTOS
	SemaphoreHandle_t m_lock;
#endif
#ifdef USE_THREADX
	TX_MUTEX m_lock;
#endif
	void * m_data;
};

struct filesystem_t {
	struct kobj_t * kobj;
	struct list_head list;
	const char * name;
    int (*mount)(struct vfs_mount_t *, const char *);
	int (*unmount)(struct vfs_mount_t *);
    size_t (*read)(struct vfs_node_t *, size_t, void *, size_t);
	size_t (*write)(struct vfs_node_t *, size_t, void *, size_t);
	int16_t (*ioctl)(struct vfs_node_t *, uint16_t, void *);
	int (*readdir)(struct vfs_node_t *, int64_t, struct vfs_dirent_t *);
	int (*lookup)(struct vfs_node_t *, const char *, struct vfs_node_t *);
};

extern struct list_head __filesystem_list;

struct filesystem_t * search_filesystem(const char * name);
uint8_t register_filesystem(struct filesystem_t * fs);
uint8_t unregister_filesystem(struct filesystem_t * fs);

void vfs_force_unmount(struct vfs_mount_t * m);
int vfs_mount(const char * dev, const char * dir, const char * fsname, uint32_t flags);
int vfs_unmount(const char * path);
struct vfs_mount_t * vfs_mount_get(int index);
int vfs_mount_count(void);
int vfs_open(const char * path, uint32_t flags);
int vfs_close(int fd);
size_t vfs_lseek(int fd, size_t offset, int whence);
size_t vfs_read(int fd, void * buf, size_t len);
size_t vfs_write(int fd, void * buf, size_t len);
int16_t vfs_ioctl(int fd, uint16_t cmd, void *buf);
int vfs_fstat(int fd, struct vfs_stat_t * st);
int vfs_opendir(const char * name);
int vfs_closedir(int fd);
int vfs_readdir(int fd, struct vfs_dirent_t * dir);
int vfs_rewinddir(int fd);
int vfs_unlink(const char * path);
int vfs_access(const char * path, uint32_t mode);
int vfs_stat(const char * path, struct vfs_stat_t * st);

void do_init_vfs(void);

void do_auto_mount(void);

#ifdef __cplusplus
}
#endif

#endif /* __VFS_H__ */
