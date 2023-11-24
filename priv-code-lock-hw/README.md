# Steps - Hardware
## Emulator

RISCV toolchin envroment variable setting.
```
mkdir -p tools
export RISCV=`pwd`/tools
cd ../
```
In roket-tools folder.

```
cd rocket-tools
git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
git checkout 64879b2



./build.sh
```
In freedom folder

```
cd freedom/rocket-chip/emulator
make -j`nproc`
```
emulator test
```
In emulator folder
make test-pcl 
```

## FPGA
1.  you need to vivado 2018.2 (Xilinx)
2. Read FPGA_manual.md

```
source <vivado 2018.2>
export RISCV=<install>
time make -f Makefile.vcu118-u500devkit verilog
time make -f Makefile.vcu118-u500devkit mcs
```

- verilog: 2m (muse)
- mcs: 55m (muse) 

Generated Address Map
                 0 -       1000 ARWX  debug-controller@0
              3000 -       4000 ARWXC error-device@3000
             10000 -      12000  R X  rom@10000
           2000000 -    2010000 ARW   clint@2000000
           c000000 -   10000000 ARW   interrupt-controller@c000000
          40000000 -   60000000  RWX  pci@2000000000
          64000000 -   64001000 ARW   serial@64000000
          64001000 -   64002000 ARW   spi@64001000
          64002000 -   64003000 ARW   gpio@64002000
          80000000 -  100000000  RWXC memory@80000000
        2000000000 - 2004000000  RW   pci@2000000000

# Dependencies and imported

- rocket-tools
- https://github.com/freechipsproject/rocket-tools
- e2c6d1577a75f506fe992c3eb20a75174504476e


# Note

Currently rocket-tools is not maintained nicely.
At least things work.




# Commit hashes

- rocket-tools: e2c6d1577a75f506fe992c3eb20a75174504476e
- rocket-tools/riscv-tests: 19bfdab48c2a6da4a2c67d5779757da7b073811d

# Config
- devkit configs @ src/main/scala/unleashed
-


# Kernel dev flow.

- Clone and make changes on host.
- Build on VM (Vagrantfile)
    - sre: 2.8G after submodule init


- need to enable kernel module

- The module flow failed: risc-v kernel may not support kernel module. (We are inevitably using an old version).

# From manual



- After the FPGA is powered and configured from BPI linear flash, the Coreplex boots Linux+root filesystem from the SD Card.
- The Freedom U500 Software Development Kit provides everything required to compile and customize this SD Card image - GCC 6.1.0 cross compilation toolchain, Linux 4.6.2 kernel and buildroot managed root filesystem.
## from privCodeLock-sw
```
To prepare a complete SD card image :
cd freedom-u-sdk; make
```
### and then get bbl.bin

To load the compiled image to the SD card :
dd if=work/bbl.bin of=/dev/your-sd-card bs=1M
