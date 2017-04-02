all:
	svn up
	cd Caravel && make clean && make
	cd Main && make clean && make

clean:
	cd Caravel && make clean
	cd Main && make clean

main:
	cd Main && ./TestMain

bs:
	ps aux|grep Build|grep -v grep|awk '{print "kill -9 "$$2}'|/bin/bash

ipcrm:
	ipcs -m|grep -v 0x00666666|grep -v Shared|grep -v key|awk '{if($$1!=""){print "ipcrm -M "$$1}}'|/bin/bash