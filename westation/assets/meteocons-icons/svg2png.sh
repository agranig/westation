#!/bin/sh

H=160

mkdir -p PNG
for i in SVG/*.svg; do
    base=$(basename "${i%.*}")
    nogrid="SVG/${base}-nogrid.svg"
    xmlstarlet ed -N ns=http://www.w3.org/2000/svg \
        -d "//ns:g[contains(@id,'Grid_1_')]" "$i" > "$nogrid"
    rsvg-convert -h $H "$nogrid" > PNG/"$base".png
done
