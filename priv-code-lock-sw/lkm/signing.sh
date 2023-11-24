#!/bin/sh
cd $1
make
echo "file position $PWD"

cd ../../work/riscv-linux
scripts/sign-file sha256 certs/signing_key.pem certs/signing_key.x509 ../../lkm/$1/$1.ko
