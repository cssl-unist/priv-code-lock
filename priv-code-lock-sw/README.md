# This repository is privCodeLock 

## Tested on ubuntu 18.04 

# Check before install
		
1. if you use python3 as python.

error: too few arguments to function '_PyImport_FixupBuiltin


	cd /usr/bin/
	sudo ln -sf python2 python
	After this you should be able to build freedom-u-sdk without any trouble. When you're done building, you should reset the symlink back to its original state:

	cd /usr/bin/
	sudo ln -sf python3 python

2. Bulidroot error
	```
	 toolchain-external undefined Configuring
	Incorrect selection of kernel headers: expected 4.6.x, got 4.13.x
	```

	- Solution: Hacky but works:in buildroot/support/scripts/check-kernel-headers.sh change the line return 1; -> return 0;

=====================  
# Install
## Install riscv-gnu-toolchain
You can go https://github.com/riscv-collab/riscv-gnu-toolchain.git
1. checkout b4dae89 hash (v20171107)
2. git submodule init
3. Some submodule were broken from riscv archive.
	- riscv-qemu also was broken -> git rm riscv-qemu (We didn't use it)
	-  if riscv-glibc was broken ->  
	go to https://github.com/riscvarchive/riscv-glibc.git and clone.
	checkout 2f626de717

4. git submodule udpate --recurisive
5. change privCodeLock_root's path
	```
	make $(privCodeLock_root)/toolchain
	 ./configure --prefix=$(privCodeLock_root)/toolchain
	make sim 
	```
# on privCodeLock 
For turn on privCodeLock, you use  syscall function (There is script in freedom-u-sdk/test-user/test) - You move this script to buildroot/system/skeleton/usr/

test script
` -a: running all test
 -e: enable pcl
 -l: list_test
 -t: test_id
-r: read_victim
`

## test
-- ID: root / password: sifive 

. cd usr/
. ./test -e
. ./test -a



# Some tips
If you have newly built Linux, or if there is a change in the lkm module, make cleanB 
 make cleanB (clean buildroot)  

If you want to build new linux. 
make cleanLinux(clean Linux)   

If you want to clean all work dir.
 make clean (clean work dir)   

# LKM signing
1. move freedom-u-sdk/lkm
	(make + signing)
	` ./signing.sh "file name"`
	For example, ./signing.sh hello (Move hello.ko to buildroot)   
2. move_buildroot_.sh( Move buildroot)- hello.ko/ greeter.ko

** (1+2)--> using one-shot.sh **

3. buildroot/device_table.txt info
	- The example is root folder - device_table.txt
	
3. In freedom-u-sdk folder, make -j48 sim
4. Need to about 10 miniutes(Depending on system status, It is different. 

5. `login: root / passward: sifive 
check ../usr`
## module load test 

After login,
cd ../usr

insmod hello.ko

log: 

# insmod hello.ko
insmod hello.ko
[DEBUG] isolation sw on! 1
[  159.467790] hello: loading out-of-tree module taints kernel.
[  159.469075] Hello world 1.
[DEBUG] isolation sw on! 0
 


