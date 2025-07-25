#pragma once

#include "common.h"
#include "list.h"

#define PROCS_MAX 8

#define PROC_FREE 0    // Free process descriptor
#define PROC_READY 1   // Ready/runnable process
#define PROC_ACTIVE 2  // Currently active process
#define PROC_BLOCKED 3 // Blocked process

#define KSTACK_SIZE 8 * 1024 // 8KB

// Process descriptor
typedef struct {
	int pid;		     // Process id
	int state;		     // Process state: UNUSED or RUNNABLE
	vaddr_t sp;		     // Stack pointer
	uint8_t kstack[KSTACK_SIZE]; // Kernel stack

	list_head list; // Intrusive linkage for scheduling
} process;

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

#define READ_CSR(reg)                                                          \
	({                                                                     \
		unsigned long __tmp;                                           \
		__asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));          \
		__tmp;                                                         \
	})

#define WRITE_CSR(reg, value)                                                  \
	do {                                                                   \
		uint32_t __tmp = (value);                                      \
		__asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));        \
	} while (0);

typedef struct {
	uint32_t ra;
	uint32_t gp;
	uint32_t tp;

	uint32_t t0;
	uint32_t t1;
	uint32_t t2;
	uint32_t t3;
	uint32_t t4;
	uint32_t t5;
	uint32_t t6;

	uint32_t s0;
	uint32_t s1;
	uint32_t s2;
	uint32_t s3;
	uint32_t s4;
	uint32_t s5;
	uint32_t s6;
	uint32_t s7;
	uint32_t s8;
	uint32_t s9;
	uint32_t s10;
	uint32_t s11;

	uint32_t a0;
	uint32_t a1;
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;

	uint32_t sp;
} __attribute__((packed)) trap_frame;

typedef struct {
	long error;
	long value;
} sbiret;
