#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    version\r\n");
}

static int do_version(int argc, char ** argv)
{
	struct machine_t * mach = get_machine();
	int i;

	for(i = 0; i < 5; i++)
		printf("%s\r\n", xboot_character_logo_string(i));
	printf("%s - [%s][%s]\r\n", xboot_banner_string(), mach ? mach->name : "unknown", mach ? mach->desc : "none");
	return 0;
}

static struct command_t cmd_version = {
	.name	= "version",
	.desc	= "show xboot version information",
	.usage	= usage,
	.exec	= do_version,
};

static void version_cmd_init(void)
{
	register_command(&cmd_version);
}

static void version_cmd_exit(void)
{
	unregister_command(&cmd_version);
}
