#include "defs.h"
#include "kernel.h"
#include "vga.h"

#include "keyboard.h"
#include "buddyalloc.h"
#include "physpgalloc.h"
#include "pgalloc.h"


static int help(int argc, char **argv);
static int testargs(int argc, char **argv);
static int echo(int argc, char **argv);

#define fwrap(f, name)				\
	static int				\
	name(int argc, char **argv)		\
	{					\
		f();				\
		return 0;			\
	}

fwrap(vga_clear_screen, clear)
fwrap(pginfo, freev)
fwrap(physpginfo, freep)
fwrap(balloc_info, freea)

#undef fwrap

static struct {
	char *cmd;
	int (*func)(int, char **);
} cmds[] = {
#define comm(c, f) { .cmd = c, .func = f },
	comm("help", help)
	comm("test", testargs)
	comm("echo", echo)
	comm("clear", clear)
	comm("freev", freev)
	comm("freep", freep)
	comm("freea", freea)
};
#undef comm

static int
help(int argc, char **argv)
{
	int i;

	for (i = 0; i < NELEMS(cmds); i++)
		iprintf("%s\t", cmds[i].cmd);
	iprintf("\n");

	return 0;
}

static int
testargs(int argc, char **argv)
{
	int i;

	iprintf("cmd = \"%s\"\n", argv[0]);

	for (i = 1; i < argc; i++)
		iprintf("\targv[%d] = \"%s\"\n", i, argv[i]);

	return 0;
}

static int
echo(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
		iprintf("%s ", argv[i]);
	iprintf("\n");

	return 0;
}

/* Return 1 if got last arg of func */
static int
getarg(char *s, size_t slen)
{
	char c;
	int i;

	i = 0;

	do
		c = getchar();
	while (c == ' ' || c == '\t');

	if (c == '\n') {
		s[0] = '\0';
		return 1;
	}

	do {
		if (c == '\b')
			i = i ? i - 1 : 0;
		else
			s[i++] = c;
		c = getchar();
	} while (c != ' ' && c != '\n' && c != '\t' && i < slen - 1);

	s[i] = '\0';
	ungetchar(c);

	return 0;
}

static int
is_cmd(char *cmd)
{
	int i;

	for (i = 0; i < NELEMS(cmds); i++)
		if (strcmp(cmd, cmds[i].cmd) == 0)
			return 1;
	return 0;
}

static void
runcmd(int argc, char **argv)
{
	int i;

	for (i = 0; i < NELEMS(cmds); i++) {
		if (strcmp(argv[0], cmds[i].cmd) == 0) {
			cmds[i].func(argc, argv);
			return;
		}
	}
}

void
shell()
{
	char argv[50][50];
	char *args[50];
	int argc, i;

	iprintf("\n");

	for (;;) {
		argc = 0;
		iprintf(">> ");

		while (getarg(argv[argc], sizeof(argv[0])) != 1
		       && argc < NELEMS(argv))
			argc++;

		for (i = 0; i < argc; i++)
			args[i] = argv[i];

		if (is_cmd(argv[0]))
			runcmd(argc, args);
		else if (strcmp(argv[0], "") != 0) {
			iprintf("Unknown cmd \"%s\"!\n", argv[0]);
			iprintf("Commands listed here:\n");
			help(0, NULL);
		}
	}
}

