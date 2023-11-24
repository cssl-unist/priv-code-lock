#!/bin/sh
## 
## echo "602.gcc_s"
## date
## ./sgcc_base.riscv-64 t1.c -O3 -finline-limit=50000 \
##   -o t1.opts-O3_-finline-limit_50000.s > t1.opts-O3_-finline-limit_50000.out 2>> t1.opts-O3_-finline-limit_50000.err
## diff t1.opts-O3_-finline-limit_50000.s t1.opts-O3_-finline-limit_50000.s.ref
## 
cd spec-cpu-2017
echo "605.mcf_s"
date
./mcf_s_base.riscv-64 inp.in  > inp.out 2>> inp.err
diff inp.out inp.out.ref
diff mcf.out mcf.out.ref

#echo "631.deepsjeng_s"
#date
#./deepsjeng_s_base.riscv-64 test.txt > test.out 2>> test.err
#diff test.out test.out.ref
