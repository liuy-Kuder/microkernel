#ifndef  __SYSFS_H_
#define  __SYSFS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "microkernel.h"
#include "vfs.h"

void filesystem_sys_init(void);
void filesystem_sys_exit(void);

uint64_t sys_write_test(struct vfs_node_t * n, int64_t off, void * buf, uint64_t len);

#ifdef __cplusplus
}
#endif

#endif
