#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    clear\r\n");
}

static int do_clear(int argc, char ** argv)
{
	printf("\033[2J");
	return 0;
}

static struct command_t cmd_clear = {
	.name	= "clear",
	.desc	= "clear the terminal screen",
	.usage	= usage,
	.exec	= do_clear,
};

static void clear_cmd_init(void)
{
	register_command(&cmd_clear);
}

static void clear_cmd_exit(void)
{
	unregister_command(&cmd_clear);
}
