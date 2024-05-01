# This repository is priv-code-lock-sw

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

# Install
## Install riscv-gnu-toolchain
We need to install toolchain/bin/riscv64-unknown-linux-gnu-gcc.


You can go https://github.com/riscv-collab/riscv-gnu-toolchain.git
1. checkout b4dae89 hash (v20171107)
2. git submodule init
3. Some submodule were broken from riscv archive.
	- riscv-qemu also was broken -> git rm riscv-qemu (We didn't use it)
	-  if riscv-glibc was broken ->  
	go to https://github.com/riscvarchive/riscv-glibc.git and clone.
	checkout 2f626de717

4. git submodule udpate --recurisive
5. cd riscv-gnu-toolchain
	```
	 ./configure --prefix=$(priv-code-lock_sw_root)/toolchain
	make linux
	```
# on privCodeLock
** Unset RISCV environment variable. 

# Run simulator and make work/bbl.bin
```
cd priv-code-lock-sw
make sim
```
-- ID: root / password: sifive 


# priv-code-lcok test
For turn on priv-code-lock, you use  syscall function (There is script in freedom-u-sdk/test-user/test) - You move this script to buildroot/system/skeleton/usr/


##  Test
```
cd usr/   
 ./test -e   
 ./test -a   
```
### Test script arguments
```
-a: running all test  
-e: enable pcl  
-l: list_test   
-t: test_id  
-r: read_victim  

```
# LKM Test    
## LKM signing
1. In lkm folder  
`cd lkm`
2. make and sign lkm module  
` ./signing.sh <file name>`   
	- For example, ./signing.sh hello      
3. Move buildroot lkm module   
`./move_buildroot_.sh`  
	- This script copies hello.ko to the buildroot filesystem.   
	- Script that does steps 2 and 3 at once --> `./one-shot.sh` 

4. Add info to buildroot/system/device_table.txt
Rmove comment on line 21
```
 1 # See package/makedevs/README for details
  2 #
  3 # This device table is used to assign proper ownership and permissions
  4 # on various files. It doesn't create any device file, as it is used
  5 # in both static device configurations (where /dev/ is static) and in
  6 # dynamic configurations (where devtmpfs, mdev or udev are used).
  7 #
  8 # <name>                                <type>  <mode>  <uid>   <gid>   <major> <minor> <start> <inc>   <count>
  9 /dev                                    d       755     0       0       -       -       -       -       -
 10 /tmp                                    d       1777    0       0       -       -       -       -       -
 11 /etc                                    d       755     0       0       -       -       -       -       -
 12 /root                                   d       700     0       0       -       -       -       -       -
 13 /var/www                                d       755     33      33      -       -       -       -       -
 14 /etc/shadow                             f       600     0       0       -       -       -       -       -
 15 /etc/passwd                             f       644     0       0       -       -       -       -       -
 16 /etc/network/if-up.d                    d       755     0       0       -       -       -       -       -
 17 /etc/network/if-pre-up.d                d       755     0       0       -       -       -       -       -
 18 /etc/network/if-down.d                  d       755     0       0       -       -       -       -       -
 19 /etc/network/if-post-down.d             d       755     0       0       -       -       -       -       -
 20 /etc/modules            f       500     0       0       -       -       -       -       -
 21 /usr/hello.ko          f       500     0       0       -       -       -       -       -
 22 # uncomment this to allow starting x as non-root
 23 #/usr/X11R6/bin/Xfbdev                  f       4755    0       0       -       -       -       -       -
~
```

	

5. In priv-code-lock-sw folder.   
    `make -j48 sim`
6. Need to about 10 miniutes (Depending on system status, It is different.  
 `login: root / passward: sifive`
## module load test 

After login,
```
cd ../usr
insmod hello.ko
```
### log: 
[DEBUG] isolation sw on! 1  
[  159.467790] hello: loading out-of-tree module taints kernel.  
[  159.469075] Hello world 1.  
[DEBUG] isolation sw on! 0  
 

# Some tips
If you have newly built Linux, or if there is a change in the lkm module, make cleanB 
`make cleanB` (clean buildroot)  

If you want to build new linux. 
`make cleanLinux`(clean Linux)   

If you want to clean all work dir.
 `make clean` (clean work dir)   



