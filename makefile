MCFLAGS += -target x86_64-pc-win32-coff

CFLAGS += -fsanitize=undefined -fno-sanitize=alignment -fsanitize-trap=all -fno-sanitize-recover=all
CFLAGS += -Wall -Wextra -O2 -ggdb
CFLAGS += -fno-delete-null-pointer-checks -ftrivial-auto-var-init=pattern
CFLAGS += -Warray-bounds -fno-strict-overflow -fno-strict-aliasing -fstrict-flex-arrays=3
CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers -Wno-int-conversion
CFLAGS += -Wconditional-uninitialized -Wunreachable-code -Wthread-safety
CFLAGS += -mno-stack-arg-probe -fno-stack-check -fno-stack-protector -fshort-wchar -mno-red-zone -mno-avx
CFLAGS += -ffreestanding

TARGET = BOOTX64

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DIS  := $(SRCS:.c=.dis)

INCS += -I ./gnu-efi/inc

LDFLAGS += -filealign:16 -subsystem:efi_application -nodefaultlib -dll -debug -entry:efi_main

OVMF_FD ?= /usr/share/ovmf/x64/OVMF.4m.fd

all: $(TARGET) dis image

$(TARGET): $(OBJS)
	lld-link $(LDFLAGS) $^ -out:$@.EFI

%.o: %.c
	clang $(MCFLAGS) $(CFLAGS) $(INCS) -c $< -o $@

dis: $(DIS)
	llvm-objdump -S -d $(TARGET).EFI > $(TARGET).dis

%.dis: %.o
	llvm-objdump -S	-d $< > $@

image: $(TARGET)
	mkdir -pv root
	mkdir -pv input/EFI/BOOT
	rsync -a --exclude 'input/*' \
		--exclude 'images/*' \
		./ input/EFI/BOOT/
	genimage --loglevel 0
	lz4 -f -k -v -12 images/disk.img

#		-nographic \

qemu: all
	qemu-system-x86_64 \
		-M q35 \
		-m 256 \
		-cpu qemu64 \
		-vga virtio \
		-device virtio-rng-pci \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_FD),readonly=on \
		-hda images/disk.img \
		-s \

clean:
	rm -rf *.dis *.su *.out *.exe *.efi *.EFI *.o *.lib *.map ./tmp/* ./input/* ./images/*
