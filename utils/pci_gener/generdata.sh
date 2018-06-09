#!/bin/sh

ROOT=$(dirname $0)/
IN=$ROOT/pci.ids
OUT=$ROOT/../../src/pci_db.c

wget http://pci-ids.ucw.cz/v2.2/pci.ids -q -O "$IN" || cp "$IN.bak" "$IN"
cp "$IN" "$IN.bak"

if [ -e $OUT -a $(md5sum $IN|cut -d' ' -f1) = $(md5sum $IN.bak|cut -d' ' -f1) ]
then
	exit 0
fi

$ROOT/struct.awk < "$IN" > $ROOT/out
N=$(wc -l $ROOT/out |cut -d' ' -f1)

echo '#include "defs.h"' > $OUT
echo '#include "pci.h"' >> $OUT
echo >> $OUT
echo "int pci_db_n = $N;">> $OUT
echo "struct pci_linux pci_db[$N] = {" >> $OUT
cat $ROOT/out >> $OUT
echo "};" >> $OUT

