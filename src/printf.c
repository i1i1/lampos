#include <stdarg.h>
#include <limits.h>

#include "kernel.h"
#include "com.h"
#include "vga.h"


#define FMT_JUSTIFY_RIGHT	0
#define FMT_JUSTIFY_LEFT	1

#define LMOD_SIGNED		BIT(4)
#define LMOD_INT		0
#define LMOD_CHAR		1
#define LMOD_SHORT		2
#define LMOD_LONG		3
#define LMOD_LONG_LONG		4
#define LMOD_SIZET		5
#define LMOD_VOIDPTR		6

#define is_ascii_digit(x)	((x) >= '0' && (x) <= '9')

static int
do_printf(const char *fmt, va_list ap, char *buf, size_t len)
{
	char *oldbuf;
	int base, conv, justify, lmod;
       	int minwidth, prec;
	int space, sign, zeropad;
	unsigned int cnt;

	cnt = 0;
	oldbuf = buf;

#define PUTBUF(c) \
do { \
	if (len > 0) { \
		*buf++ = c; \
		len--; \
	} \
	cnt++; \
} while (0)

	while (*fmt != '\0') {

		if (*fmt != '%') {
			PUTBUF(*fmt);
			fmt++;
			continue;
		}

		/* proceed to next format string character */
		fmt++;

		/* user just wants to print a percent sign */
		if (*fmt == '%') {
			PUTBUF('%');
			fmt++;
			continue;
		}

		space = 0;
		zeropad = 0;
		base = 0;
		sign = 0;
		justify = FMT_JUSTIFY_RIGHT;
		minwidth = -1;
		prec = -1;
		lmod = LMOD_INT;

		/* Obtain optional flags, which can appear in any order. */
		while (*fmt != '\0') {
			if (*fmt == '-')
				justify = FMT_JUSTIFY_LEFT;
			else if (*fmt == '+')
				sign = 1;
			else if (*fmt == ' ')
				space = 1;
			else if (*fmt == '0') {
				zeropad = 1;
			}
			else
				break;
			fmt++;
		}

		/* Obtain optional minimum field width. */
		if (*fmt == '*') {
			minwidth = va_arg(ap, int);
			if (minwidth < 0)
				justify = FMT_JUSTIFY_LEFT;
			fmt++;
		} else if (is_ascii_digit(*fmt)) {
			minwidth = 0;
			do {
				minwidth = 10*minwidth + *fmt - '0';
				fmt++;
			} while (*fmt && is_ascii_digit(*fmt));
		}

		/* Obtain optional precision. */
		if (*fmt == '.') {
			fmt++;
			if (*fmt == '*') {
				prec = va_arg(ap, int);
				if (prec < 0)
					prec = 0;
				fmt++;
			} else if (is_ascii_digit(*fmt)) {
				prec = 0;
				do {
					prec = 10*prec + *fmt - '0';
					fmt++;
				} while (*fmt && is_ascii_digit(*fmt));
			}
		}

		/* Get length modifiers. */
		if (fmt[0] == 'h') {
			if (fmt[1] == 'h') {
				lmod = LMOD_SHORT;
				fmt += 2;
			} else {
				lmod = LMOD_CHAR;
				fmt++;
			}
		} else if (fmt[0] == 'l') {
			if (fmt[1] == 'l') {
				lmod = LMOD_LONG_LONG;
				fmt += 2;
			} else {
				lmod = LMOD_LONG;
				fmt++;
			}
		} else if (*fmt == 'z') {
			lmod = LMOD_SIZET;
			fmt++;
		}

		if (*fmt == 'd' || *fmt == 'i') {
			base = 10;
			lmod |= LMOD_SIGNED;
		} else if (*fmt == 'u') {
			base = 10;
		} else if (*fmt == 'o') {
			base = 8;
		} else if (*fmt == 'x' || *fmt == 'X') {
			base = 16;
		} else if (*fmt == 'p') {
			base = 16;
			lmod = LMOD_VOIDPTR;
		} else if (*fmt == 's') {
			base = -1;
		} else if (*fmt == 'c') {
			base = -2;
		} else
			continue;

		conv = *fmt;
		fmt++;

		if (base > 0) {
			char dig[32];
			unsigned int width, minus, zero, nzeros;
			char *cp;
			unsigned long long val;
			char *p1, *p2, tmp, padchr, sgnchr;

			switch (lmod) {
			case LMOD_CHAR | LMOD_SIGNED: val = (char)va_arg(ap, int); break;
			case LMOD_SHORT | LMOD_SIGNED: val = (short)va_arg(ap, int); break;
			case LMOD_INT | LMOD_SIGNED: val = va_arg(ap, int); break;
			case LMOD_LONG | LMOD_SIGNED: val = va_arg(ap, long); break;
			case LMOD_LONG_LONG | LMOD_SIGNED: val = va_arg(ap, long long); break;
			case LMOD_CHAR: val = (unsigned char)va_arg(ap, int); break;
			case LMOD_SHORT: val = (unsigned short)va_arg(ap, int); break;
			case LMOD_INT: val = va_arg(ap, unsigned int); break;
			case LMOD_LONG: val = va_arg(ap, unsigned long); break;
			case LMOD_LONG_LONG: val = va_arg(ap, unsigned long long); break;
			case LMOD_SIZET: val = va_arg(ap, size_t); break;
			case LMOD_VOIDPTR: val = (unsigned long)va_arg(ap, void *); break;
			default: val = va_arg(ap, int); break;
			}

			cp = dig;
			width = 0;
			minus = 0;
			sgnchr = 0;
			zero = 0;
			nzeros = 0; /* additional zeros to appear in conversion */

			/*
			 * Produce printable representation of digits
			 * one by one in reverse order.
			 */

			if (base == 10 && conv != 'u' && (long long)val < 0) {
				minus = 1;
				val = -((long long)val);
			}

			if (val == 0) {
				zero = 1;
				if (conv == 'p') {
					const char *nulstr = "(nul)";
					const char *p = nulstr;
					while (*p != '\0')
						*cp++ = *p++;
					*cp = '\0';
					width = strlen(nulstr);
					zeropad = 0;
					space = 0;
					goto skip_reverse;
				} else {
					/*
					 * When precision is explicitely specified as
					 * zero, the result of conversion shall be
					 * no characters.
					 */
					if (prec == 0)
						continue;
					width = 1;
					*cp++ = '0';
					*cp = '\0';
				}
			}

			while (val > 0) {
				int c;
				c = val % base;
				val /= base;
				if (c >= 0 && c <= 9)
					*cp++ = c + '0';
				else
					*cp++ = (c - 10) + (conv == 'X' ? 'A' : 'a');
				width++;
			}

			*cp = '\0';

			/* Calculate how many zeros to append to conversion, when precision specified. */
			if (prec >= 0 && width < prec) {
				nzeros = prec - width;
				width += nzeros;
			}

			/* Reverse printable representation of digits. */
			p1 = dig, p2 = cp - 1;
			while (p1 < p2) {
				tmp = *p1;
				*p1++ = *p2;
				*p2-- = tmp;
			}

		skip_reverse:
			/*
			 * Ignore zero padding when precision or left justify flags present.
			 */
			if (justify == FMT_JUSTIFY_LEFT || prec >= 0)
				zeropad = 0;

			/* Ignore explicit sign for anything except signed decimal conversion. */
			if (base != 10 || conv == 'u')
				sign = 0;

			/* Ignore space when sign flag specified or non-decimal conversion base. */
			if (sign || base != 10)
				space = 0;

			/* Select padding character. */
			padchr = zeropad ? '0' : ' ';

			/* Add sign to decimal conversion if needed. */
			if (sign || minus) {
				if (minus) {
					sgnchr = '-';
					space = 0;
				} else if (sign) {
					sgnchr = '+';
				}
			}

			/* Correct field width before outputing characters to the user's buffer. */
			if (sgnchr != 0 || space)
				width++;
			if (conv == 'p' && !zero)
				width += 2;

			if (justify == FMT_JUSTIFY_RIGHT) {

				/* Prepend `0x' prefix to `p' conversion when zero padding field. */
				if (conv == 'p' && !zero && zeropad) {
					PUTBUF('0');
					PUTBUF('x');
				}

				/* Fill the field to minimum width with padding character . */
				if (minwidth >= 0) {
					for (; width < minwidth; width++)
						PUTBUF(padchr);
				}

				/* Append <space>, `+' or `-', or `0x' prefix to conversion. */
				if (space)
					PUTBUF(' ');
				else if (sgnchr)
					PUTBUF(sgnchr);
				else if (conv == 'p' && !zero && !zeropad) {
					PUTBUF('0');
					PUTBUF('x');
				}

				if (conv != 'p')
					for (; nzeros > 0; nzeros--)
						PUTBUF('0');

				for (cp = dig; *cp != '\0'; cp++)
					PUTBUF(*cp);
			} else {

				if (space)
					PUTBUF(' ');
				else if (sgnchr)
					PUTBUF(sgnchr);
				else if (conv == 'p' && !zero) {
					PUTBUF('0');
					PUTBUF('x');
				}

				if (conv != 'p')
					for (; nzeros > 0; nzeros--)
						PUTBUF('0');

				for (cp = dig; *cp != '\0'; cp++)
					PUTBUF(*cp);

				/* Pad the field with space characters. */
				if (minwidth >= 0) {
					for (; width < minwidth; width++)
						PUTBUF(' ');
				}
			}

		} else if (base == -1) {
			size_t width, n;
			char *cp;

			/*
			 * Output NUL-terminated string to user's buffer.
			 */
			cp = va_arg(ap, char *);
			n = strlen(cp);

			if (prec >= 0)
				width = n > prec ? prec : n;
			else
				width = n;

			if (justify == FMT_JUSTIFY_RIGHT)
				goto output_spad;
		output_str:
			if (prec >= 0) {
				for (; prec && *cp != '\0'; prec--, cp++)
					PUTBUF(*cp);
			} else {
				for (; *cp != '\0'; cp++)
					PUTBUF(*cp);
			}

			if (justify == FMT_JUSTIFY_RIGHT)
				continue;
		output_spad:
			if (minwidth > 0) {
				for (; width < minwidth; width++)
					PUTBUF(' ');
			}

			if (justify == FMT_JUSTIFY_RIGHT)
				goto output_str;

		} else if (base == -2) {
			unsigned char c;
			c = (unsigned char)va_arg(ap, int);
			PUTBUF(c);
		}

	}

	if (len > 0)
		*buf = '\0';
	else if (buf != oldbuf)
		buf[-1] = '\0';

	return cnt;
}

int
isnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = do_printf(fmt, ap, buf, size);
	va_end(ap);

	return res;
}

int
iprintf(const char *fmt, ...)
{
	char buf[2048];
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = do_printf(fmt, ap, buf, sizeof(buf));

	com_puts(COM1_PORT_ADDRESS, buf);
	vga_puts(buf);
	va_end(ap);

	return res;
}

