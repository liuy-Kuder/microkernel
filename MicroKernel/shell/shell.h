#ifndef __SHELL_H__
#define __SHELL_H__

#ifdef __cplusplus
extern "C" {
#endif

int shell_realpath(const char * path, char * fpath);
const char * shell_getcwd(void);
int shell_setcwd(const char * path);
int shell_system(const char * cmdline);
int vmexec(const char * path, const char * fb, const char * input);

void shell_task(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_H__ */
