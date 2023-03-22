#!/bin/bash

FILE=minimap2_opencl

cd lib

rm -f $FILE.aocx
aoc ../src/fpga/$FILE.cl -o $FILE.aocx -board=pac_a10 -report
mv $FILE.aocx $FILE.tmp.aocx
printf 'Y\nY\n' | $AOCL_BOARD_PACKAGE_ROOT/linux64/libexec/sign_aocx.sh -H openssl_manager -i $FILE.tmp.aocx -r NULL -k NULL -o $FILE.aocx
rm $FILE.tmp.aocx

