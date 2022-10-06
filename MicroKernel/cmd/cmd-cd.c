#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    cd [dir]\r\n");
}

static int do_cd(int argc, char ** argv)
{
	const char * path;

	if(argc == 1)
		path = "/";
	else
		path = argv[1];

	if(shell_setcwd(path) < 0)
	{
		printf("cd: %s: No such file or directory\r\n", path);
		return -1;
	}
	return 0;
}

static struct command_t cmd_cd = {
	.name	= "cd",
	.desc	= "change the current working directory",
	.usage	= usage,
	.exec	= do_cd,
};

void cd_cmd_init(void)
{
	register_command(&cmd_cd);
}

void cd_cmd_exit(void)
{
	unregister_command(&cmd_cd);
}
