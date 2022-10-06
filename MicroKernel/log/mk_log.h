#ifndef __MK_LOG_H_
#define __MK_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "microkernel.h"

/*Possible log level. For compatibility declare it independently from `MK_USE_LOG`*/

#define MK_LOG_LEVEL_TRACE 0 /**< A lot of logs to give detailed information*/
#define MK_LOG_LEVEL_INFO  1 /**< Log important events*/
#define MK_LOG_LEVEL_WARN  2 /**< Log if something unwanted happened but didn't caused problem*/
#define MK_LOG_LEVEL_ERROR 3 /**< Only critical issue, when the system may fail*/
#define MK_LOG_LEVEL_USER  4 /**< Custom logs from the user*/
#define MK_LOG_LEVEL_NONE  5 /**< Do not log anything*/
#define _MK_LOG_LEVEL_NUM  6 /**< Number of log levels */

typedef int8_t mk_log_level_t;

#ifdef MK_USE_LOG

/**
 * Log print function. Receives "Log Level", "File path", "Line number", "Function name" and "Description".
 */
typedef void (*mk_log_print_g_cb_t)(mk_log_level_t level, const char *, uint32_t, const char *, const char *);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param print_cb a function pointer to print a log
 */
void mk_log_register_print_cb(mk_log_print_g_cb_t print_cb);

/**
 * Add a log
 * @param level the level of log. (From `MK_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _mk_log_add(mk_log_level_t level, const char * file, int line, const char * func, const char * format, ...);

/**********************
 *      MACROS
 **********************/

#if MK_LOG_LEVEL <= MK_LOG_LEVEL_TRACE
#define MK_LOG_TRACE(...) _mk_log_add(MK_LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define MK_LOG_TRACE(...) do {}while(0)
#endif

#if MK_LOG_LEVEL <= MK_LOG_LEVEL_INFO
#define MK_LOG_INFO(...) _mk_log_add(MK_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define MK_LOG_INFO(...) do {}while(0)
#endif

#if MK_LOG_LEVEL <= MK_LOG_LEVEL_WARN
#define MK_LOG_WARN(...) _mk_log_add(MK_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define MK_LOG_WARN(...) do {}while(0)
#endif

#if MK_LOG_LEVEL <= MK_LOG_LEVEL_ERROR
#define MK_LOG_ERROR(...) _mk_log_add(MK_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define MK_LOG_ERROR(...) do {}while(0)
#endif

#if MK_LOG_LEVEL <= MK_LOG_LEVEL_USER
#define MK_LOG_USER(...) _mk_log_add(MK_LOG_LEVEL_USER, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define MK_LOG_USER(...) do {}while(0)
#endif

#else /*MK_USE_LOG*/

/*Do nothing if `MK_USE_LOG 0`*/
#define _mk_log_add(level, file, line, ...)
#define MK_LOG_TRACE(...) do {}while(0)
#define MK_LOG_INFO(...)  do {}while(0)
#define MK_LOG_WARN(...)  do {}while(0)
#define MK_LOG_ERROR(...) do {}while(0)
#define MK_LOG_USER(...)  do {}while(0)
#endif /*MK_USE_LOG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
