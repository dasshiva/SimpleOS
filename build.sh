set -e
clang -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -c src/*.c -I ../inc/
clang -target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link *.o -o BOOTX64.EFI
dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT
mcopy -i fat.img kernel.wasm ::
mcopy -i fat.img font.psf ::
qemu-system-x86_64  -pflash OVMF.fd -usb \
                                   -drive if=none,id=stick,format=raw,file=fat.img \
                                   -device nec-usb-xhci,id=xhci \
                                   -device usb-storage,bus=xhci.0,drive=stick 
rm *.o
