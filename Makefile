QEMU=qemu-system-riscv32
QEMU_FLAGS=-machine virt -bios default -nographic -serial mon:stdio --no-reboot
CC=clang
CFLAGS=-std=c11 -O3 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib
OBJDUMP=llvm-objdump

IMAGE=kernel.elf
OUTPUTS=$(IMAGE) kernel.map

$(OUTPUTS) : kernel.ld kernel.c
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o $(IMAGE) kernel.c

.PHONY: run
run : $(IMAGE)
	echo "Hint: 'Ctrl-a + x' to terminate QEMU"
	$(QEMU) $(QEMU_FLAGS) -kernel $(IMAGE)

.PHONY: clean
clean:
	rm $(OUTPUTS)

.PHONY: objdump
objdump:
	$(OBJDUMP) -d $(IMAGE) | less
