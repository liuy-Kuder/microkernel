#ifndef  __MICROKERNEL_H_
#define  __MICROKERNEL_H_

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "./core/kobj.h"
#include "./core/mk_driver.h"
#include "./core/mk_device.h"
#include "./core/mk_list.h"
#include "./core/mk_setup.h"
#include "./filesystem/sysfs.h"
#include "./filesystem/vfs.h"

#include "rtos.h"

//#include "cmsis_iccarm.h"

/***************************
 * CURRENT VERSION OF LVGL
 ***************************/
#define LVGL_VERSION_MAJOR 0
#define LVGL_VERSION_MINOR 1
#define LVGL_VERSION_PATCH 1
#define LVGL_VERSION_INFO ""

//is enable log
#define MK_USE_LOG
#define MK_LOG_PRINTF   1
#define MK_LOG_LEVEL    MK_LOG_LEVEL_INFO

#define ARRAY_SIZE(array)	( sizeof(array) / sizeof((array)[0]) )

#define MICROKERNEL_MAX_DEVICES             (64)

#define CONFIG_DEVICE_HASH_SIZE				(64)
#define CONFIG_DRIVER_HASH_SIZE				(64)

typedef enum { FALSE = 0, TRUE = 1 } boolean;

#endif
