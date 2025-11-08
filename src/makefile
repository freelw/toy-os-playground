all: link liblansys bins
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
	dd if=lan_os of=a.img bs=512 count=62 skip=8 seek=1 conv=notrunc
	head -c 1474560 /dev/zero > a.vfd
	dd if=a.img of=a.vfd bs=512 count=63 conv=notrunc
	node fs_patch_tool/patch.js
link: head lan_main boot
	ld -m elf_i386 -T n.lds -o lan_os \
	 head.o lan_main.o sys_call.o intel_err_handle.o \
	 sched.o fork.o mm.o print_str.o keyboard.o exit.o
head:
	nasm -felf32 -o head.o head.s
lan_main:
	gcc -O0 -m32 -g -fno-stack-protector -fno-pie \
	-c lan_main.c sys_call.c intel_err_handle.c sched.c \
	fork.c mm.c print_str.c keyboard.c exit.c \
	-Wno-incompatible-pointer-types \
	-Wno-implicit-function-declaration \
	-Wno-int-conversion
liblansys:
	$(MAKE) -C common/lib
liblansys_clean:
	$(MAKE) -C common/lib clean
bins:
	$(MAKE) -C bin/loop
	$(MAKE) -C bin/ls
	$(MAKE) -C bin/shell
	$(MAKE) -C bin/test
	$(MAKE) -C bin/loop cp
	$(MAKE) -C bin/ls cp
	$(MAKE) -C bin/shell cp
	$(MAKE) -C bin/test cp
bins_clean:
	$(MAKE) -C bin/loop clean
	$(MAKE) -C bin/ls clean
	$(MAKE) -C bin/shell clean
	$(MAKE) -C bin/test clean
root_fs_clean:
	-rm root_fs/loop
	-rm root_fs/ls
	-rm root_fs/lan_sh
	-rm root_fs/test
boot:
	nasm -o boot.bin boot.s -l boot.lst
clean: liblansys_clean bins_clean root_fs_clean
	-rm lan_os lan_main.o sys_call.o \
	intel_err_handle.o head.o sched.o fork.o \
	mm.o print_str.o keyboard.o exit.o \
	boot.bin boot.lst a.img a.vfd
	