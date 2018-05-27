#!/bin/sh

W=160

mkdir -p PNG
for i in SVG/*.svg; do
    qlmanage -t -s $W -o PNG/ $i
done
