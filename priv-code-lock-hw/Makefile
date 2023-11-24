.phony: program

pull-bbl:
	vagrant scp default:~/freedom-u-sdk/work/bbl.bin .

program:
	vivado -mode batch -source program.tcl
