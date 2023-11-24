

test: test.c
	riscv64-unknown-linux-gnu-gcc $< -static -o $@

.PHONY: install
install: test drive.sh
	cp test ../work/buildroot_initramfs_sysroot/root/
	cp drive.sh ../work/buildroot_initramfs_sysroot/root/
	cp run-lm.sh ../work/buildroot_initramfs_sysroot/root/
	cp victim.txt ../work/buildroot_initramfs_sysroot/root/
	#rm -rf ../work/buildroot_initramfs_sysroot/root/lmbench3 && cd lmbench3 && make -j6 && cd .. && cp -r lmbench3 ../work/buildroot_initramfs_sysroot/root/ 
	#rm -rf ../work/buildroot_initramfs_sysroot/root/spec-cpu-2017  && cp -r spec-cpu-2017 ../work/buildroot_initramfs_sysroot/root/

clean:
	rm -f test
