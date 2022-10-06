#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    pwd\r\n");
}

static int do_pwd(int argc, char ** argv)
{
	printf("%s\r\n", shell_getcwd());
	return 0;
}

static struct command_t cmd_pwd = {
	.name	= "pwd",
	.desc	= "print the current working directory",
	.usage	= usage,
	.exec	= do_pwd,
};

void pwd_cmd_init(void)
{
	register_command(&cmd_pwd);
}

void pwd_cmd_exit(void)
{
	unregister_command(&cmd_pwd);
}
