#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    echo [option] [string]...\r\n");
	printf("    -n    do not output the trailing newline\r\n");
}

static int do_echo(int argc, char ** argv)
{
	int nflag = 0;

	if(*++argv && !strcmp(*argv, "-n"))
	{
		++argv;
		nflag = 1;
	}

	while(*argv)
	{
		printf("%s", *argv);
		if(*++argv)
			putchar(' ');
	}

	if(nflag == 0)
		printf("\r\n");
	fflush(stdout);

	return 0;
}

static struct command_t cmd_echo = {
	.name	= "echo",
	.desc	= "echo the string to standard output",
	.usage	= usage,
	.exec	= do_echo,
};

static void echo_cmd_init(void)
{
	register_command(&cmd_echo);
}

static void echo_cmd_exit(void)
{
	unregister_command(&cmd_echo);
}
