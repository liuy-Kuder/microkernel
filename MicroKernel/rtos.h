//兼容各类操作系统或裸机
#ifndef  __RTOS_H_
#define  __RTOS_H_

#define USE_OS
#define USE_FREERTOS
//#define USE_THREADX

#ifdef USE_OS

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

    #elif #define USE_THREADX
        #if 0
        sample:
        uint8_t MKPoolBuf[1024*30];//30k
        TX_BYTE_POOL MK_pool;

            /* 创建内存池，类似malloc和free */    
            tx_byte_pool_create(&MK_pool, 
                    "MK_pool",
                    (VOID *)MKPoolBuf,     /* 内存池地址，需要保证4字节对齐 */
                    sizeof(MKPoolBuf));    /* 内存池大小 */
        #endif

        #include "tx_api.h"
        #define mutex_lock(x)       tx_mutex_get(&x,TX_WAIT_FOREVER)
        #define mutex_unlock(x)     tx_mutex_put(&x)
        #define mutex_init(x)       tx_mutex_create(&x,"vfs_mutex",1)
        #define spin_lock_irq()     TX_INTERRUPT_SAVE_AREA; TX_DISABLE
        #define spin_unlock_irq()   TX_RESTORE
        extern TX_BYTE_POOL MK_pool;
        static void * tx_malloc(size_t len)//二次封装
        {
            void  *Ptr;
            uint8_t r = tx_byte_allocate(&MK_pool,&Ptr,len,TX_NO_WAIT);
            if(r != TX_SUCCESS)
            {
                Ptr = NULL;
            }
            return Ptr;
        }
        #define MK_MALLOC(X)    tx_malloc(X)
        #define MK_FREE(X)      tx_byte_release(X)
    #endif

#else //NO USE_OS
    #define mutex_unlock(x)   do{}while(0)
    #define mutex_lock(x)     do{}while(0)

    #define spin_lock_irq()                   //  __disable_irq()
    #define spin_unlock_irq()                 //  __enable_irq()

    #define MK_MALLOC(X)    malloc(X)
    #define MK_FREE(X)      free(X)
#endif

#endif
