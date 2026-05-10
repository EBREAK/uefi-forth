MCFLAGS += -target x86_64-pc-win32-coff

CFLAGS += -Wno-incompatible-pointer-types-discards-qualifiers -Wno-int-conversion
CFLAGS += -Wall -Wextra -O2
CFLAGS += -mno-stack-arg-probe -fno-stack-check -fno-stack-protector -fshort-wchar -mno-red-zone
CFLAGS += -mno-avx -fno-strict-aliasing -ffreestanding 

TARGET := BOOTX64.EFI

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DIS  := $(SRCS:.c=.dis)

INCS += -I ./gnu-efi/inc

LDFLAGS += -filealign:16 -subsystem:efi_application -nodefaultlib -dll -entry:efi_main

OVMF_FD ?= /usr/share/ovmf/x64/OVMF.4m.fd

all: $(TARGET) image

$(TARGET): $(OBJS)
	lld-link $(LDFLAGS) $^ -out:$@

%.o: %.c
	clang $(MCFLAGS) $(CFLAGS) $(INCS) -c $< -o $@

dis: $(DIS)
%.dis: %.o
	llvm-objdump -s -d $< > $@

image: $(TARGET)
	mkdir -pv root
	mkdir -pv input/EFI/BOOT
	rsync -a --exclude 'input/*' \
		--exclude 'images/*' \
		./ input/EFI/BOOT/
	genimage --loglevel 0


qemu: image
	qemu-system-x86_64 \
		-M q35 \
		-m 256 \
		-cpu qemu64 \
		-nographic \
		-device virtio-rng-pci \
		-drive if=pflash,format=raw,unit=0,file=$(OVMF_FD),readonly=on \
		-hda images/disk.img

clean:
	rm -rf *.dis *.su *.out *.exe *.efi *.EFI *.o *.lib *.map ./tmp/* ./input/* ./images/*
