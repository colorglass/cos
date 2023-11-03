toolchain ?= ~/opt/cross/bin
prefix ?= i686-elf

CC = $(toolchain)/$(prefix)-gcc
CC_FLAGS = -nostdlib -ffreestanding -Wall -I include

.phony: all
all: disk.img
disk.img: boot.bin kernel.elf
	truncate -s 1M disk.img
	truncate -s 64M part_main		# fat32 has a min size of 32M
	mkfs.fat -F 32 part_main
	cat part_main >> disk.img		
	rm part_main
	truncate -s +512 disk.img		# the last sector of the disk seems to not be used in fdisk partition
	fdisk disk.img < fdisk.txt
	dd if=boot.bin of=disk.img conv=notrunc bs=440 count=1
	dd if=boot.bin of=disk.img conv=notrunc bs=512 skip=1 seek=1
	sudo mount -o loop,offset=1048576 disk.img temp
	sudo cp kernel.elf temp
	sudo umount temp

boot.bin: boot/*.S boot/*.c
	$(CC) $(CC_FLAGS) -T boot/bootloader.ld $^ -o boot.a -g
	objcopy -O binary boot.a $@

kernel.elf: kernel/*.c
	$(CC) $(CC_FLAGS) -T kernel/kernel.ld -o $@ $^ -g

.phony: debug
debug: disk.img boot.a
	qemu-system-i386 -s -S -display curses -hda disk.img -m 128M

.phony: clean
clean:
	rm *.a *.bin *.img *.elf