set here=`pwd`
eval `make -f ${here}/Makefile setup_csh | egrep -v '^LCGREL='`
