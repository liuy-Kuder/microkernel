/********************************************************************
*
*文件名称：mk_log.c
*内容摘要：提供日志功能
*当前版本：V1.0
*作者：刘杨
*完成时期：2022.10.3
*其他说明: none
*
**********************************************************************/

#include "mk_log.h"
#ifdef MK_USE_LOG

#include <stdarg.h>
#include <string.h>

#if LV_LOG_PRINTF
    #include <stdio.h>
#endif

static mk_log_print_g_cb_t custom_print_cb;

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param print_cb a function pointer to print a log
 */
void mk_log_register_print_cb(mk_log_print_g_cb_t print_cb)
{
    custom_print_cb = print_cb;
}
//uint8_t myuserinfo[1025];
/**
 * Add a log
 * @param level the level of log. (From `mk_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _mk_log_add(mk_log_level_t level, const char * file, int line, const char * func, const char * format, ...)
{
    if(level >= _MK_LOG_LEVEL_NUM)
     return; /*Invalid level*/

    if(level >= MK_LOG_LEVEL) 
     {
        va_list args;
        va_start(args, format);
        char buf[256];
        snprintf(buf, sizeof(buf), format, args);
        va_end(args);

#if MK_LOG_PRINTF
        /*Use only the file name not the path*/
        size_t p;
        for(p = strlen(file); p > 0; p--)
         {
            if(file[p] == '/' || file[p] == '\\')
             {
                p++;    /*Skip the slash*/
                break;
             }
         }

        static const char * lvl_prefix[] = {"Trace", "Info", "Warn", "Error", "User"};
//        sprintf(myuserinfo,"%s: %s \t(%s #%d %s())\n", lvl_prefix[level], buf, &file[p], line, func);
        printf("%s: %s (%s #%d %s())\r\n", lvl_prefix[level], buf, &file[p], line, func);
#else
        if(custom_print_cb) custom_print_cb(level, file, line, func, buf);
#endif
     }
}

#endif
