#!/bin/sh
./signing.sh hello
./signing.sh greeter
cd greeter  
cp greeter.ko ../../buildroot/system/skeleton/usr/
cd ../hello
cp hello.ko ../../buildroot/system/skeleton/usr/
echo "Done"


