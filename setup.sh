here=`pwd`
eval `make -f ${here}/Makefile setup_sh | egrep -v '^LCG(APP|EXT)='`
