#include "kernel.h"
#include "common.h"
#include "list.h"

// Linker variables
extern char __bss_start[], __bss_end[];
extern char __stack_top[];
extern char __free_memory_start[], __free_memory_end[];

// Processes
process procs[PROCS_MAX];    // all process descriptors
LIST_HEAD(procs_free);	     // free processes
LIST_HEAD(procs_ready);	     // ready processes
process *proc_active = NULL; // active process
LIST_HEAD(procs_blocked);    // blocked processes

// Special processes
process kernel_proc = {0};
process *idle_proc = NULL;

// SBI call
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

long putchar(int ch) {
	sbiret r = sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);
	return r.error; // legacy extensions have return value in a0
}

__attribute__((naked)) // ensure compiler adds nothing to start/end of function
__attribute__((aligned(4))) // align to 4 byte boundary
void kernel_entry(void) {
	// Save all the general purpose (x1-x31) registers to stack
	__asm__ __volatile__(
	    "csrw sscratch, sp\n" // store stack pointer to supervisor scratch
				  // register
	    "addi sp, sp, -4 * 31\n" // prepare to push 31 elements to stack (32
				     // registers minus program counter)
	    "sw ra, 4 * 0(sp)\n"     // return address
	    "sw gp, 4 * 1(sp)\n"     // global pointer
	    "sw tp, 4 * 2(sp)\n"     // thread pointer

	    "sw t0, 4 * 3(sp)\n" // temporary registers (caller-saved?)
	    "sw t1, 4 * 4(sp)\n"
	    "sw t2, 4 * 5(sp)\n"
	    "sw t3, 4 * 6(sp)\n"
	    "sw t4, 4 * 7(sp)\n"
	    "sw t5, 4 * 8(sp)\n"
	    "sw t6, 4 * 9(sp)\n"

	    "sw s0, 4 * 10(sp)\n" // callee-saved registers
	    "sw s1, 4 * 11(sp)\n"
	    "sw s2, 4 * 12(sp)\n"
	    "sw s3, 4 * 13(sp)\n"
	    "sw s4, 4 * 14(sp)\n"
	    "sw s5, 4 * 15(sp)\n"
	    "sw s6, 4 * 16(sp)\n"
	    "sw s7, 4 * 17(sp)\n"
	    "sw s8, 4 * 18(sp)\n"
	    "sw s9, 4 * 19(sp)\n"
	    "sw s10, 4 * 20(sp)\n"
	    "sw s11, 4 * 21(sp)\n"

	    "sw a0, 4 * 22(sp)\n" // argument registers
	    "sw a1, 4 * 23(sp)\n"
	    "sw a2, 4 * 24(sp)\n"
	    "sw a3, 4 * 25(sp)\n"
	    "sw a4, 4 * 26(sp)\n"
	    "sw a5, 4 * 27(sp)\n"
	    "sw a6, 4 * 28(sp)\n"
	    "sw a7, 4 * 29(sp)\n"

	    "csrr a0, sscratch\n" // read previously saved stack pointer to a0
	    "sw a0, 4 * 30(sp)\n" // save sp to stack
	);

	// Call handle_trap, with argument as the stack pointer
	__asm__ __volatile__("mv a0, sp\n"
			     "call handle_trap\n");

	// Load registers back from stack in same order as before
	__asm__ __volatile__(
	    "lw ra, 4 * 0(sp)\n" // return address
	    "lw gp, 4 * 1(sp)\n" // global pointer
	    "lw tp, 4 * 2(sp)\n" // thread pointer

	    "lw t0, 4 * 3(sp)\n" // temporary registers (caller-saved?)
	    "lw t1, 4 * 4(sp)\n"
	    "lw t2, 4 * 5(sp)\n"
	    "lw t3, 4 * 6(sp)\n"
	    "lw t4, 4 * 7(sp)\n"
	    "lw t5, 4 * 8(sp)\n"
	    "lw t6, 4 * 9(sp)\n"

	    "lw s0, 4 * 10(sp)\n" // callee-saved registers
	    "lw s1, 4 * 11(sp)\n"
	    "lw s2, 4 * 12(sp)\n"
	    "lw s3, 4 * 13(sp)\n"
	    "lw s4, 4 * 14(sp)\n"
	    "lw s5, 4 * 15(sp)\n"
	    "lw s6, 4 * 16(sp)\n"
	    "lw s7, 4 * 17(sp)\n"
	    "lw s8, 4 * 18(sp)\n"
	    "lw s9, 4 * 19(sp)\n"
	    "lw s10, 4 * 20(sp)\n"
	    "lw s11, 4 * 21(sp)\n"

	    "lw a0, 4 * 22(sp)\n" // argument registers
	    "lw a1, 4 * 23(sp)\n"
	    "lw a2, 4 * 24(sp)\n"
	    "lw a3, 4 * 25(sp)\n"
	    "lw a4, 4 * 26(sp)\n"
	    "lw a5, 4 * 27(sp)\n"
	    "lw a6, 4 * 28(sp)\n"
	    "lw a7, 4 * 29(sp)\n"

	    "lw sp, 4 * 30(sp)\n" // stack pointer
	);

	// Return to where exception was raised
	__asm__ __volatile__("sret"); // supervisor return
}

void handle_trap(trap_frame *f) {
	(void)f;
	uint32_t scause = READ_CSR(scause);
	uint32_t stval = READ_CSR(stval);
	uint32_t user_pc = READ_CSR(sepc);

	PANIC("unexpected trap scause=0x%x, stval=0x%x, sepc=0x%x\n", scause,
	      stval, user_pc);
}

// Context switch between procs
__attribute__((naked)) //
void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
	__asm__ __volatile__(
	    "addi sp, sp, -4 * 13\n" // prepare to push 13 elements to stack
	    "sw ra, 4 * 0(sp)\n"     // return addr
	    "sw s0, 4 * 1(sp)\n"     // callee-saved registers
	    "sw s1, 4 * 2(sp)\n"
	    "sw s2, 4 * 3(sp)\n"
	    "sw s3, 4 * 4(sp)\n"
	    "sw s4, 4 * 5(sp)\n"
	    "sw s5, 4 * 6(sp)\n"
	    "sw s6, 4 * 7(sp)\n"
	    "sw s7, 4 * 8(sp)\n"
	    "sw s8, 4 * 9(sp)\n"
	    "sw s9, 4 * 10(sp)\n"
	    "sw s10, 4 * 11(sp)\n"
	    "sw s11, 4 * 12(sp)\n"

	    "sw sp, (a0)\n" // save sp to arg0 (prev_sp)
	    "lw sp, (a1)\n" // load sp from arg1 (next_sp)

	    "lw ra, 4 * 0(sp)\n" // return addr
	    "lw s0, 4 * 1(sp)\n" // callee-saved registers
	    "lw s1, 4 * 2(sp)\n"
	    "lw s2, 4 * 3(sp)\n"
	    "lw s3, 4 * 4(sp)\n"
	    "lw s4, 4 * 5(sp)\n"
	    "lw s5, 4 * 6(sp)\n"
	    "lw s6, 4 * 7(sp)\n"
	    "lw s7, 4 * 8(sp)\n"
	    "lw s8, 4 * 9(sp)\n"
	    "lw s9, 4 * 10(sp)\n"
	    "lw s10, 4 * 11(sp)\n"
	    "lw s11, 4 * 12(sp)\n"
	    "addi sp, sp, 4 * 13\n" // pop 13 elements from stack

	    "ret\n" // return
	);
}

process *create_process(uint32_t init_fn) {
	static int last_pid = 0;

	// Find an unused process
	process *proc = list_first_entry_or_null(&procs_free, process, list);

	if (proc == NULL) {
		PANIC("ran out of process descriptors");
	}
	assert(proc != NULL);

	list_del(&proc->list);

	// Initialize stack
	uint32_t *sp = (uint32_t *)&proc->kstack[sizeof(proc->kstack)];
	*(--sp) = 0;	   // s11
	*(--sp) = 0;	   // s10
	*(--sp) = 0;	   // s9
	*(--sp) = 0;	   // s8
	*(--sp) = 0;	   // s7
	*(--sp) = 0;	   // s6
	*(--sp) = 0;	   // s5
	*(--sp) = 0;	   // s4
	*(--sp) = 0;	   // s3
	*(--sp) = 0;	   // s2
	*(--sp) = 0;	   // s1
	*(--sp) = 0;	   // s0
	*(--sp) = init_fn; // ra

	// Initialize fields
	proc->pid = ++last_pid;
	proc->state = PROC_READY;
	proc->sp = (vaddr_t)sp;

	// Move to ready queue
	list_add_tail(&proc->list, &procs_ready);

	return proc;
}

// Allocates `n` 4KB pages and returns the starting address
paddr_t alloc_pages(uint32_t n) {
	static paddr_t next_paddr = (paddr_t)__free_memory_start;
	paddr_t paddr = next_paddr;
	next_paddr += n * PAGE_SIZE;

	// Check if we have enough memory
	if (next_paddr > (paddr_t)__free_memory_end) {
		PANIC("out of memory");
	}

	memset((void *)paddr, 0, n * PAGE_SIZE);

	assert(is_aligned(paddr, PAGE_SIZE));
	return paddr;
}

void delay(void) {
	for (int i = 0; i < 300000000; i++) {
		__asm__ __volatile__("nop");
	}
}

process *proca;
process *procb;

void proca_main(void) {
	for (;;) {
		printf("Hello from %s!\n", __func__);
		switch_context(&proca->sp, &kernel_proc.sp);
		delay();
	}
}

void procb_main(void) {
	for (;;) {
		printf("Hello from %s!\n", __func__);
		switch_context(&procb->sp, &kernel_proc.sp);
		delay();
	}
}

void init_procs(void) {
	// Clear procs array
	memset(procs, 0, sizeof(procs));
}

void init_scheduler(void) {
	// Add each process to the procs_free list
	for (int i = 0; i < PROCS_MAX; i++) {
		list_head *new = &procs[i].list;
		list_head *head = &procs_free;
		list_add(new, head);
	}
}

void init(void) {
	init_procs();	  // setup procs struct
	init_scheduler(); // setup scheduler
}

void schedule(void) {
	if (proc_active != NULL) {
		list_add_tail(&proc_active->list, &procs_ready);
		proc_active = NULL;
	}
	assert(proc_active == NULL);

	// Remove from ready queue & mark as active task
	assert(!list_empty(&procs_ready));
	proc_active = list_first_entry_or_null(&procs_ready, process, list);
	list_del(&proc_active->list);

	// Switch context from kernel to active task
	assert(proc_active != NULL);
	switch_context(&kernel_proc.sp, &proc_active->sp);
}

void kmain(void) {
	// clear bss section
	memset(__bss_start, 0, (size_t)__bss_end - (size_t)__bss_start);

	// setup exception vector
	WRITE_CSR(stvec, (uint32_t)kernel_entry);

	// initialization
	init();

	printf("\n\nKernel Booted! Built at %s on %s\n\n", __TIME__, __DATE__);

	proca = create_process((uint32_t)proca_main);
	procb = create_process((uint32_t)procb_main);

	// Scheduling loop
	for (;;) {
		schedule();
	}

	// switch_context(&kernel_proc.sp, &proca->sp);
	// proca_main();

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
