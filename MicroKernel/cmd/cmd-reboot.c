#include "command.h"

static void usage(void)
{
	printf("usage:\r\n");
	printf("    reboot\r\n");
}

static int reboot(int argc, char ** argv)
{
	//machine_reboot();
	return 0;
}

static struct command_t cmd_reboot = {
	.name	= "reboot",
	.desc	= "reboot the target system",
	.usage	= usage,
	.exec	= reboot,
};

void reboot_cmd_init(void)
{
	register_command(&cmd_reboot);
}

void reboot_cmd_exit(void)
{
	unregister_command(&cmd_reboot);
}
