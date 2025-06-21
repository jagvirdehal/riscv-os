QEMU:=qemu-system-riscv32
QEMU_FLAGS:=-machine virt -bios default -nographic -serial mon:stdio --no-reboot
CC:=clang
CFLAGS:=-std=c11 -O3 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fno-stack-protector -ffreestanding -nostdlib -MMD -MP
LDFLAGS:=-Wl,-Tkernel.ld -Wl,-Map=kernel.map
OBJDUMP:=llvm-objdump

IMAGE:=kernel.elf
OUTPUTS:=$(IMAGE) kernel.map

# Source files and include dirs
SRC_DIRS := ./src
BUILD_DIR := ./build

# Get source files
SOURCES := $(shell find $(SRC_DIRS) -type f -name '*.c')

# Convert source file to object/depend files
OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# Find all directories in src folder
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add -I before each folder
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Build image
$(IMAGE) : $(OBJECTS) kernel.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(IMAGE) $(filter-out %.ld, $^)

# Build C source files
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

# Utilities
.PHONY: run clean objdump
run : $(IMAGE)
	echo -e "\n\n ======================================\n" \
	     " Hint: 'Ctrl-a + x' to terminate QEMU\n" \
	     "======================================\n"
	$(QEMU) $(QEMU_FLAGS) -kernel $(IMAGE)
clean:
	rm $(OUTPUTS)
	rm -r $(BUILD_DIR)
objdump:
	$(OBJDUMP) -d $(IMAGE) | less

-include $(DEPENDS)
