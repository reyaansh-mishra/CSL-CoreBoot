qemu-system-aarch64 \
  	-machine virt,virtualization=on,secure=off \
  	-cpu max \
	-m 10G \
	-drive if=pflash,format=raw,readonly=on,file=EFI.fd \
	-drive if=pflash,format=raw,file=NVRAM.fd \
	-drive format=raw,file=fat:rw:esp \
	-device qemu-xhci,id=xhci -device usb-kbd,bus=xhci.0 \
	-d int,cpu_reset,guest_errors \
	-D qemu.log \
	-device virtio-gpu-pci \
	-nographic \
	-s
	
