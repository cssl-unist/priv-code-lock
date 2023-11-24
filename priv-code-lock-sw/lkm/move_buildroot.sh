#!/bin/sh
set -e

cd greeter  
make
cp greeter.ko ../../buildroot/system/skeleton/usr/
cd ../hello
make
cp hello.ko ../../buildroot/system/skeleton/usr/
echo "Done"



