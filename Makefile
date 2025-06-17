QEMU=qemu-system-riscv32
QEMU_FLAGS=-machine virt -bios default -nographic -serial mon:stdio --no-reboot

CC=clang
CFLAGS=-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib

kernel.map kernel.elf : kernel.ld kernel.c
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf kernel.c

.PHONY: run
run : kernel.elf
	echo "Hint: 'Ctrl-a + x' to terminate QEMU"
	$(QEMU) $(QEMU_FLAGS) -kernel kernel.elf
