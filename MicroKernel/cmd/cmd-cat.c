#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    cat <file> ...\r\n");
}

static int cat_file(const char * filename)
{
	struct vfs_stat_t st;
	char fpath[VFS_MAX_PATH];
	char * buf;
	u64_t i, n;
	int fd;

	if(shell_realpath(filename, fpath) < 0)
	{
		printf("cat: %s: Can not convert to realpath\r\n", filename);
		return -1;
	}

	if(vfs_stat(fpath, &st) < 0)
	{
		printf("cat: %s: No such file or directory\r\n", fpath);
		return -1;
	}

	if(S_ISDIR(st.st_mode))
	{
		printf("cat: %s: Is a directory\r\n", fpath);
		return -1;
	}

	fd = vfs_open(fpath, O_RDONLY, 0);
	if(fd < 0)
	{
		printf("cat: %s: Can not open\r\n", fpath);
		return -1;
	}

	buf = malloc(SZ_64K);
	if(!buf)
	{
		printf("cat: Can not alloc memory\r\n");
		vfs_close(fd);
		return -1;
	}

	while((n = vfs_read(fd, buf, SZ_64K)) > 0)
	{
		for(i = 0; i < n; i++)
		{
			if(isprint(buf[i]) || (buf[i] == '\r') || (buf[i] == '\n') || (buf[i] == '\t') || (buf[i] == '\f'))
				putchar(buf[i]);
			else
				putchar('.');
		}
	}
	printf("\r\n");

	free(buf);
	vfs_close(fd);

	return 0;
}

static int do_cat(int argc, char ** argv)
{
	int i;

	if(argc == 1)
	{
		usage();
		return -1;
	}

	for(i = 1; i < argc; i++)
	{
		if(cat_file(argv[i]) != 0)
			return -1;
	}
	return 0;
}

static struct command_t cmd_cat = {
	.name	= "cat",
	.desc	= "show the contents of a file",
	.usage	= usage,
	.exec	= do_cat,
};

static void cat_cmd_init(void)
{
	register_command(&cmd_cat);
}

static void cat_cmd_exit(void)
{
	unregister_command(&cmd_cat);
}
