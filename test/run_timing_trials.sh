#!/bin/bash
for i in `seq 1 9`; do
    echo "===== ${i}000 flows =====";
    head -n ${i}000 flows.txt.core > flows.txt;
    ./flowconflict.py;
    echo;
done
