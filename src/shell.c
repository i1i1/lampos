#include "defs.h"
#include "kernel.h"
#include "vga.h"

#include "keyboard.h"
#include "buddyalloc.h"
#include "physpgalloc.h"
#include "pgalloc.h"


struct args {
	int c;
	char v[20][50];
};

static int help(struct args *arg);
static int testargs(struct args *arg);
static int echo(struct args *arg);
static int clear(struct args *arg);
static int exit(struct args *arg);

#define fwrap(f, name)		\
	static int				\
	name(struct args *arg)	\
	{						\
		f();				\
		return 0;			\
	}

fwrap(pginfo, freev)
fwrap(physpginfo, freep)
fwrap(balloc_info, freea)
#undef fwrap


static struct {
	char *cmd;
	int (*func)(struct args *);
} cmds[] = {
#define comm(c, f) { .cmd = c, .func = f },
	comm("help", help)
	comm("test", testargs)
	comm("echo", echo)
	comm("clear", clear)
	comm("freev", freev)
	comm("freep", freep)
	comm("freea", freea)
	comm("exit", exit)
};
#undef comm

static int
exit(struct args *arg)
{
	outb(0xf4, 0x00);
	return 0;
}

static int
help(struct args *arg)
{
	int i;

	for (i = 0; i < NELEMS(cmds); i++)
		iprintf("%s\t", cmds[i].cmd);
	iprintf("\n");

	return 0;
}

static int
testargs(struct args *arg)
{
	int i;

	iprintf("cmd = \"%s\"\n", arg->v[0]);

	for (i = 1; i < arg->c; i++)
		iprintf("\targv[%d] = \"%s\"\n", i, arg->v[i]);

	return 0;
}

static int
echo(struct args *arg)
{
	int i;

	for (i = 1; i < arg->c; i++)
		iprintf("%s ", arg->v[i]);
	iprintf("\n");

	return 0;
}

static int
clear(struct args *arg)
{
	iprintf("\f");

	return 0;
}

static void
getcmd(struct args *arg)
{
	char buf[100];
	int i, e;
	char c;

	i = 0;
	while ((c = getchar()) != '\n' && i + 1 < NELEMS(buf)) {
		if (c == '\b' && i == 0)
			continue;
		/* Ignore tab for now */
		if (c == '\t')
			continue;
		vga_putc(c);
		if (c == '\b')
			i--;
		else
			buf[i++] = c;
	}

	e = i;
	buf[e] = '\0';
	i = 0;
	arg->c = 0;

	while (i < e && arg->c < NELEMS(arg->v)) {
		int j;

		while (buf[i] == ' ' && i < e)
			i++;
		j = 0;
		while (buf[i] != ' ' && j + 1 < NELEMS(arg->v[0]) && i < e)
			arg->v[arg->c][j++] = buf[i++];
		arg->v[arg->c++][j] = '\0';
	}
	vga_putc('\n');
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
runcmd(struct args *arg)
{
	int i;

	for (i = 0; i < NELEMS(cmds); i++) {
		if (strcmp(arg->v[0], cmds[i].cmd) == 0) {
			cmds[i].func(arg);
			return;
		}
	}
}

void
shell()
{
	struct args arg;

	iprintf("\n");

	for (;;) {
		iprintf(">> ");
		getcmd(&arg);

		if (is_cmd(arg.v[0]))
			runcmd(&arg);
		else if (strcmp(arg.v[0], "") != 0) {
			iprintf("Unknown cmd \"%s\"!\n", arg.v[0]);
			iprintf("Commands listed here:\n");
			help(&arg);
		}
	}
}

