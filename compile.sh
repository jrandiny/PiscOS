#!/bin/bash
set -e
rm -rf build
mkdir build
cd build
dd if=/dev/zero of=floppya.img bs=512 count=2880
nasm ../bootload.asm -o bootload
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
dd if=../map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc
dd if=../files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc
dd if=../sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc
bcc -ansi -c -o kernel.o ../kernel.c
bcc -ansi -c -o string.o ../string.c
as86 ../kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o string.o
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=1
as86 ../lib.asm -o lib_asm.o
bcc -ansi -c -o echo.o ../echo.c
bcc -ansi -c -o mkdir.o ../mkdir.c
bcc -ansi -c -o ls.o ../ls.c
bcc -ansi -c -o shell.o ../shell.c
ld86 -o echo -d echo.o lib_asm.o
ld86 -o mkdir -d mkdir.o lib_asm.o
ld86 -o ls -d ls.o lib_asm.o
ld86 -o shell -d shell.o lib_asm.o
gcc ../loadFile.c -o loadFile -w
./loadFile echo
./loadFile mkdir
./loadFile ls
./loadFile shell
cp ../title.txt title.txt
cp ../logo.txt logo.txt
./loadFile logo.txt
./loadFile title.txt