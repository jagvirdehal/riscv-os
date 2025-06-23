#include "common.h"

long putchar(int ch);

void *memset(void *buf, char c, size_t n) {
	uint8_t *p = (uint8_t *)buf;
	while (n--)
		*(p++) = c;
	return buf;
}

void *memcpy(void *dst, const void *src, size_t n) {
	uint8_t *d = (uint8_t *)dst;
	const uint8_t *s = (const uint8_t *)src;
	while (n--)
		*(d++) = *(s++);
	return dst;
}

char *strcpy(char *dst, const char *src) {
	char *d = dst;
	while (*src)
		*(d)++ = *(src)++;
	*d = '\0';
	return dst;
}

char *strcpy_s(char *dst, size_t dst_sz, const char *src) {
	char *d = dst;
	while (*src && --dst_sz)
		*(d)++ = *(src)++;
	*d = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *d = dst;
	while (*src && n--)
		*(d)++ = *(src)++;
	*d = '\0';
	return dst;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2) break;
		s1++;
		s2++;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (*s1 && *s2 && n--) {
		if (*s1 != *s2) break;
		s1++;
		s2++;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	for (const char *ch = fmt; *ch != '\0'; ch++) {
		// print non-% chars as normal
		if (*ch != '%') {
			putchar(*ch);
			continue;
		}

		// handle format strs
		ch++; // skip '%'
		switch (*ch) {
			// End of string - print '%' and exit
			case '\0':
				putchar('%');
				goto end;

			// Escape for '%'
			case '%': {
				putchar(*ch);
			} break;

			// Print as a string
			case 's': {
				const char *str = va_arg(args, const char *);
				for (const char *ch = str; *ch != '\0'; ch++) {
					putchar(*ch);
				}
			} break;

			// Print as a decimal num
			case 'd': {
				int d = va_arg(args, int);
				unsigned magnitude = d;
				if (d < 0) {
					putchar('-');
					magnitude = -magnitude;
				}

				unsigned divisor = 1;
				while (magnitude / divisor > 9) {
					divisor *= 10;
				}

				while (divisor != 0) {
					unsigned digit =
					    (magnitude / divisor) % 10;
					putchar('0' + digit);
					divisor /= 10;
				}
			} break;

			// Print as a hexadecimal num
			case 'x': {
				unsigned value = va_arg(args, unsigned);
				for (int i = 7; i >= 0; i--) {
					unsigned nibble =
					    (value >> (i * 4)) & 0xf;
					putchar(
					    "0123456789abcdef"[nibble]);
				}
			} break;

			// TODO: Unsupported format str - print as-is for now
			default:
				putchar(*ch);
		}
	}

end:
	va_end(args);
}
