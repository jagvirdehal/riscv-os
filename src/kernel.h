#pragma once

#define PANIC(fmt, ...)                                                        \
	do {                                                                   \
		printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__,          \
		       ##__VA_ARGS__);                                         \
		while (1) {                                                    \
			__asm__ __volatile__("wfi");                           \
		}                                                              \
	} while (0);

#define assert(cond)                                                           \
	if (!(cond)) {                                                         \
		PANIC("Assertion error: %s evaluated to false", #cond);        \
	}

typedef struct {
	long error;
	long value;
} sbiret;
