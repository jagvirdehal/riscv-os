#include "common.h"

long sbi_console_putchar(int ch);

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	for (const char *ch = fmt; *ch != '\0'; ch++) {
		// print non-% chars as normal
		if (*ch != '%') {
			sbi_console_putchar(*ch);
			continue;
		}

		// handle format strs
		ch++; // skip '%'
		switch (*ch) {
			// End of string - print '%' and exit
			case '\0':
				sbi_console_putchar('%');
				goto end;

			// Escape for '%'
			case '%': {
				sbi_console_putchar(*ch);
			} break;

			// Print as a string
			case 's': {
				const char *str = va_arg(args, const char *);
				for (const char *ch = str; *ch != '\0'; ch++) {
					sbi_console_putchar(*ch);
				}
			} break;

			// Print as a decimal num
			case 'd': {
				int d = va_arg(args, int);
				unsigned magnitude = d;
				if (d < 0) {
					sbi_console_putchar('-');
					magnitude = -magnitude;
				}

				unsigned divisor = 1;
				while (magnitude / divisor > 9) {
					divisor *= 10;
				}

				while (divisor != 0) {
					unsigned digit =
					    (magnitude / divisor) % 10;
					sbi_console_putchar('0' + digit);
					divisor /= 10;
				}
			} break;

			// Print as a hexadecimal num
			case 'x': {
				unsigned value = va_arg(args, unsigned);
				for (int i = 7; i >= 0; i--) {
					unsigned nibble =
					    (value >> (i * 4)) & 0xf;
					sbi_console_putchar(
					    "0123456789abcdef"[nibble]);
				}
			} break;

			// TODO: Unsupported format str - print as-is for now
			default:
				sbi_console_putchar(*ch);
		}
	}

end:
	va_end(args);
}
