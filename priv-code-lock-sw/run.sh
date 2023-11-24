#!/bin/bash

dispatch() {
  case "$1" in
    scp)
      vagrant scp $2 default:/home/vagrant/freedom-u-sdk/$2
      ;;
    build-spike)
      make -j6 spike
      ;;
    spike)
      ./work/riscv-isa-sim/spike -l -p4 --isa=rv64imafdc ~/pcl-test.riscv 2> a.log
      ;;
    *)
      echo "unknown command $arg"
  esac
}

dispatch $@
