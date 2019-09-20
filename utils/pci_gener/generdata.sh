#!/bin/sh

cd $(dirname $0)

tmp=$(mktemp)

curl http://pci-ids.ucw.cz/v2.2/pci.ids | ./struct.awk >$tmp

N=$(wc -l $tmp | awk '{print $1}')

echo '#include "defs.h"'
echo '#include "pci.h"'
echo
echo "int pci_db_n = $N;"
echo "struct pci_ext_db pci_db[$N] = {"
cat $tmp
echo "};"

rm $tmp

