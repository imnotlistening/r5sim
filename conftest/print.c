/*
 * A simple implementation of printf() with a few useful argument types.
 */

#include <stdarg.h>

#include <r5sim/hw/vuart.h>

#include "conftest.h"
#include "types.h"

/*
 * Base addresses for the default machine.
 */
#define VUART_BASE	0x4000000

#define PRINTF_FLAG_ZERO_PAD		0x1
#define PRINTF_FLAG_LEFT_ADJUST		0x2

#define PRINTF_LENGTH_INT		0x0
#define PRINTF_LENGTH_CHAR		0x1
#define PRINTF_LENGTH_SHORT		0x2
#define PRINTF_LENGTH_LONG		0x3

/*
 * Sized to hold a 32 bit integer in base 2, with one NUL byte.
 */
#define PRINTF_TEMP_BUF_SIZE		33

#define MAX(a, b)				\
	(a > b ? a : b)

static const uint32_t base10[] = {
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000
};

static const char base16_lookup[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static void
__putc(char c)
{
	/*
	 * I do not know why, but I cannot get the PTY settings to be
	 * reasonable. No matter what I do I cannot get onlcr to work.
	 * As a work-around for this, just translate nl into cr-nl.
	 */
	if (c == '\n')
		writel(VUART_BASE + VUART_WRITE, '\r');

	writel(VUART_BASE + VUART_WRITE, c);
}

static void
__puts(const char *str)
{
	int i = 0;

	while (str[i] != 0)
		__putc(str[i++]);
}

static int
is_digit(char c)
{
	return c >= '0' && c <= '9';
}

static int
is_space(char c)
{
	return c == ' ' || c == '\t';
}

static void
__memset(void *__dst, int __byte, uint32_t bytes)
{
	char byte = (char)__byte;
	char *dst = (char *)__dst;
	int i;

	for (i = 0; i < bytes; i++)
		dst[i] = byte;
}

static void
__memcpy(void *__dst, const void *__src, uint32_t bytes)
{
	int i;
	char *dst = (char *)__dst;
	char *src = (char *)__src;

	for (i = 0; i < bytes; i++)
		dst[i] = src[i];
}

static uint32_t
__strlen(const char *str)
{
	int length = 0;

	while (*str++)
		length++;

	return length;
}

static uint32_t
mul10(uint32_t value)
{
	uint32_t doubled = value + value;

	return doubled + (doubled << 3);
}

static uint32_t
atou(const char *buf, int *consumed)
{
	uint32_t value = 0;

	*consumed = 0;

	/* Skip prepended whitespace. */
	while (is_space(*buf)) {
		buf++;
		(*consumed)++;
	}

	while (is_digit(*buf)) {
		value = mul10(value) + (*buf - '0');
		(*consumed)++;
		buf++;
	}

	return value;
}

static void
utostr_10(char *buf, uint32_t value)
{
	int digit = 9;
	int i = 0;

	__memset(buf, 0, PRINTF_TEMP_BUF_SIZE);

	/* Special case for 0. */
	if (value == 0) {
		buf[0] = '0';
		buf[1] = '\0';
	}

	while (base10[digit] > value)
		digit--;

	while (digit >= 0) {
		buf[i] = '0';
		while (value >= base10[digit]) {
			buf[i]++;
			value -= base10[digit];
		}

		/* Special case: the very last digit. */
		if (base10[digit] == 1 && value == 1) {
			buf[i]++;
		}

		i++;
		digit--;
	}
}

static void
utostr_16(char *buf, uint32_t value)
{
	int nibble_offs = 28;
	uint32_t nibble;
	int i = 0;

	__memset(buf, 0, PRINTF_TEMP_BUF_SIZE);

	/* Special case for 0. */
	if (value == 0) {
		buf[0] = '0';
		buf[1] = '\0';
	}

	/* Clear leading 0s. */
	while (nibble_offs >= 0) {
		nibble = (value >> nibble_offs) & 0xF;
		if (nibble)
			break;

		nibble_offs -= 4;
	}

	while (nibble_offs >= 0) {
		nibble = (value >> nibble_offs) & 0xF;
		nibble_offs -= 4;
		buf[i++] = base16_lookup[nibble];
	}

	return;
}

/*
 * Raw to string implementation; only does base 10 and base 16
 * conversions.
 *
 * Note that we have to do this without a division.
 */
static int
utostr(char *buf, uint32_t value, int base)
{
	if (base != 10 && base != 16) {
		printf("Bad base: %d\n", base);
		return -1;
	}

	if (base == 10) {
		utostr_10(buf, value);
		return 0;
	}

	utostr_16(buf, value);
	return 0;
}

static int
printf_do_conversion(char *buf, uint32_t value,
		     int base,
		     uint32_t flags,
		     uint32_t width,
		     uint32_t length)
{
	char *buf_ptr = buf;
	char raw_conv_buf[PRINTF_TEMP_BUF_SIZE];
	int max_i, conv_len, remainder;

	if (width >= PRINTF_TEMP_BUF_SIZE)
		return -1;

	switch (length) {
	case PRINTF_LENGTH_CHAR:
		value &= 0xFF;
		break;
	case PRINTF_LENGTH_SHORT:
		value &= 0xFFFF;
		break;
	case PRINTF_LENGTH_LONG:
	case PRINTF_LENGTH_INT:
		/* Nothing to do here. */
		break;
	default:
		printf("> bad length: %d\n", length);
		return -1;
	}

	/*
	 * Basic idea is to do the raw conversion into the raw_conv_buf
	 * and then copy it into buf based on flags and width.
	 */
	if (utostr(raw_conv_buf, value, base)) {
		printf("Conv failed!\n");
		return -1;
	}

	__memset(buf, 0, PRINTF_TEMP_BUF_SIZE);

	/* Work out max_i: the end of the field we are filling. */
	conv_len = __strlen(raw_conv_buf);
	max_i = MAX(width, conv_len);

	if (flags & PRINTF_FLAG_ZERO_PAD)
		__memset(buf_ptr, '0', max_i - conv_len);
	else
		__memset(buf_ptr, ' ', max_i - conv_len);

	if ((flags & PRINTF_FLAG_LEFT_ADJUST) == 0) {
		buf_ptr += max_i - conv_len;
	}

	__memcpy(buf_ptr, raw_conv_buf, conv_len);

	/* Handle any extra right side padding. */
	buf_ptr += conv_len;
	remainder = width - (buf_ptr - buf);

	if (remainder > 0) {
		__memset(buf_ptr, ' ', remainder);
		buf_ptr += remainder;
	}

	buf_ptr[0] = '\0';

	return 0;
}

static int
printf_parse_conversion(const char **fmt,
			uint32_t *flags,
			uint32_t *width,
			uint32_t *length)
{
	int consumed = 0;

	*flags = 0;
	*width = 0;
	*length = 0;

	*fmt += 1;

	/*
	 * First flags: we support '0' and '-'.
	 */
	switch (**fmt) {
	case '0':
		*flags |= PRINTF_FLAG_ZERO_PAD;
		*fmt += 1;
		break;
	case '-':
		*flags |= PRINTF_FLAG_LEFT_ADJUST;
		*fmt += 1;
		break;
	default:
		/* Hmm, not a flag it seems. */
		break;
	}

	/*
	 * Now try and parse a width (but only if there's a digit).
	 * consumed will just be 0 if there's no width so the subsequent
	 * addition becomes a noop.
	 */
	if (is_digit(**fmt))
		*width = atou(*fmt, &consumed);

	(*fmt) += consumed;

	/*
	 * Now we try to parse a length.
	 */
	switch (**fmt) {
	case 'h':
		(*fmt) += 1;
		/* Double h -> char. */
		if (*(*fmt) == 'h') {
			(*fmt) += 1;
			*length = PRINTF_LENGTH_CHAR;
		} else {
			*length = PRINTF_LENGTH_SHORT;
		}
		break;
	case 'l':
		(*fmt) += 1;
		*length = PRINTF_LENGTH_LONG;
		break;
	default:
		/* This may be ok: it could just be a conversion specifier. */
		break;
	}

	return 0;
}

int printf_print_str(const char *str, uint32_t __width, uint32_t flags)
{
	int i = 0;
	int width = (int)__width;
	int len = __strlen(str);

	if ((flags & PRINTF_FLAG_LEFT_ADJUST) == 0)
		while (i++ < (width - len))
			__putc(' ');

	__puts(str);

	if (flags & PRINTF_FLAG_LEFT_ADJUST)
		while (i++ < (width - len))
			__putc(' ');

	return width > len ? width : len;
}

int
printf(const char *fmt, ...)
{
	va_list args;
	int bytes = 0;
	uint32_t flags, width, length;
	uint32_t value;
	char buf[PRINTF_TEMP_BUF_SIZE];
	const char *str;

	va_start(args, fmt);

	while (*fmt != '\0') {
		const char c = *fmt;

		if (c != '%') {
			fmt++;
			bytes += 1;
			__putc(c);
			continue;
		}

		/*
		 * Begin a conversion; parse the flags, width, and
		 * length fields.
		 *
		 * This will update the fmt pointer to point to the
		 * conversion character - skipping all the flag, etc,
		 * stuff, and also fill in the supplied format
		 * specifier arguments. It may fail if the input is
		 * bad.
		 */
		if (printf_parse_conversion(&fmt, &flags,
					    &width, &length))
			return bytes;

		// printf("> F=0x%x; W=%u; L=0x%x\n", flags, width, length);

		/*
		 * Get the actual conversion to print.
		 */
		// printf("> Conv flag: %c\n", *fmt);
		switch (*fmt++) {
		case 'u':
			value = va_arg(args, uint32_t);
			// printf("> Value: %u\n", value);
			if (printf_do_conversion(buf, value, 10,
						 flags, width, length))
				return bytes;

			str = buf;

			break;
		case 'x':
			value = va_arg(args, uint32_t);
			if (printf_do_conversion(buf, value, 16,
						 flags, width, length))
				return bytes;

			str = buf;

			break;
		case 's':
			bytes += printf_print_str(
				va_arg(args, const char *),
				width, flags);
			str = buf;
			buf[0] = '\0';
			break;
		case '%':
			str = buf;
			buf[0] = '%';
			buf[1] = '\0';
			break;
		default:
			return bytes;
		}

		bytes += __strlen(str);
		__puts(str);
	}

	va_end(args);

	return bytes;
}