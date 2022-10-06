//兼容各类操作系统或裸机
//v0.1: 支持裸机
//v0.2: 支持threadx
#ifndef  __RTOS_H_
#define  __RTOS_H_

#ifdef USE_OS

#ifdef USE_THREADX

#define mutex_unlock(x)            
#define mutex_lock(x)               

#endif

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"

#define mutex_lock(x)       xSemaphoreTake(x, portMAX_DELAY);
#define mutex_unlock(x)     xSemaphoreGive(x)
#define mutex_init(x)       x = xSemaphoreCreateMutex()

#define spin_lock_irq()     portDISABLE_INTERRUPTS()
#define spin_unlock_irq()   portENABLE_INTERRUPTS()

#define MK_MALLOC(X)    pvPortMalloc(X)
#define MK_FREE(X)      vPortFree(X)

#endif

#ifdef USE_UCOSIII

#define mutex_unlock(x)            
#define mutex_lock(x)               

#endif
#else //NO USE_OS

#define mutex_unlock(x)   do{}while(0)
#define mutex_lock(x)     do{}while(0)

#define spin_lock_irq()                     __disable_irq()
#define spin_unlock_irq()                   __enable_irq()

#define MK_MALLOC(X)    malloc(X)
#define MK_FREE(X)      free(X)

#endif

#endif
