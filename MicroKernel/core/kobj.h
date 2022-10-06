#ifndef __KOBJ_H__
#define __KOBJ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mk_list.h"
#include <stdint.h>
  
enum kobj_type_t {
	KOBJ_TYPE_DIR,//文件
	KOBJ_TYPE_REG,//节点
};

struct kobj_t
{
	/* kobj name */
	char * name;
	/* kobj type DIR or REG */
	enum kobj_type_t type;
	/* kobj's parent */
	struct kobj_t * parent;
	/* kobj's entry */
	struct list_head entry;
	/* kobj's children */
	struct list_head children;
	/* kobj read */
	int16_t (*read)(struct kobj_t * kobj, void * buf, size_t size);
	/* kobj write */
	int16_t (*write)(struct kobj_t * kobj, void * buf, size_t size);
	/* kobj ioctl */
	int16_t (*ioctl)(struct kobj_t * kobj, uint16_t cmd, void * buf);
	/* private data */
	void * priv;
};

typedef int16_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef int16_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef int16_t (*kobj_ioctl_t)(struct kobj_t * kobj, uint16_t cmd, void * buf);

struct kobj_t * kobj_get_root(void);
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_alloc_directory(const char * name);
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, kobj_ioctl_t ioctl, void * priv);
uint8_t kobj_free(struct kobj_t * kobj);
uint8_t kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
uint8_t kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
uint8_t kobj_add_directory(struct kobj_t * parent, const char * name);
uint8_t kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, kobj_ioctl_t ioctl, void * priv);
uint8_t kobj_remove_self(struct kobj_t * kobj);
uint32_t shash(const char * s);

#ifdef __cplusplus
}
#endif

#endif /* __KOBJ_H__ */
