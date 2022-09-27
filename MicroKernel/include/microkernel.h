#ifndef  __MICROKERNEL_H_
#define  __MICROKERNEL_H_

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../core/kobj.h"
#include "../core/mk_driver.h"
#include "../core/mk_device.h"
#include "../core/mk_list.h"

#include "../filesystem/sysfs.h"
#include "../filesystem/vfs.h"
#include "rtos.h"

#include "cmsis_iccarm.h"

#define MK_VERSION_M      1
#define MK_VERSION_S      0

#define ARRAY_SIZE(array)	( sizeof(array) / sizeof((array)[0]) )

#define MICROKERNEL_MAX_DEVICES             (521)

#define CONFIG_DEVICE_HASH_SIZE				(521)
#define CONFIG_DRIVER_HASH_SIZE				(521)

#define spin_lock_irq()                     __disable_irq()
#define spin_unlock_irq()                   __enable_irq()

#define MK_OK       0
#define MK_ERROR   -1


typedef enum { FALSE = 0, TRUE = 1 } boolean;

uint32_t shash(const char * s);

#endif
