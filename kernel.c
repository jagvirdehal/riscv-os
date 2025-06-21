#include "kernel.h"
#include "common.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss_start[], __bss_end[], __stack_top[];

sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
		long arg5, long fid, long eid) {
	// Load register in quotes to variable, and assign to corresponding arg
	register long a0 __asm__("a0") = arg0;
	register long a1 __asm__("a1") = arg1;
	register long a2 __asm__("a2") = arg2;
	register long a3 __asm__("a3") = arg3;
	register long a4 __asm__("a4") = arg4;
	register long a5 __asm__("a5") = arg5;
	register long a6 __asm__("a6") = fid;
	register long a7 __asm__("a7") = eid;

	__asm__ __volatile__("ecall"
			     : "=r"(a0), "=r"(a1)
			     : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
			       "r"(a5), "r"(a6), "r"(a7)
			     : "memory");
	return (sbiret){.error = a0, .value = a1};
}

void *memset(void *buf, char c, size_t n) {
	uint8_t *p = (uint8_t *)buf;
	while (n--)
		*(p++) = c;
	return buf;
}

long sbi_console_putchar(int ch) {
	sbiret r = sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
	return r.error; // legacy extensions have return value in a0
}

void console_print(char *s) {
	for (char *ch = s; *ch != '\0'; ch++) {
		sbi_console_putchar(*ch);
	}
}

void kmain(void) {
	memset(__bss_start, 0, (size_t)__bss_end - (size_t)__bss_start);

	console_print("Hello, World!\n");

	printf("Num: %d - Hex: %x - String: %s", -10005, 255, "hello");

	for (;;) {
		__asm__ __volatile__("wfi"); // cpu sleep until interrupt
	}
}

__attribute__((section(".text.boot"))) // place function at section ".text.boot"
__attribute__((naked)) // ensure compiler adds nothing to start/end of function
void boot(void) {
	__asm__ __volatile__("mv sp, %[stack_top]\n"
			     "j kmain\n"
			     :
			     : [stack_top] "r"(__stack_top));
}
