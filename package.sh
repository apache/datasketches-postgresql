#!/bin/bash

VER=$1

DST=datasketches-$VER

PGARCH=postgres-$VER.zip
COREARCH=core-$VER.zip

rm -rf $DST
rm $DST.zip

git archive --format zip --prefix=$DST/ --output $PGARCH master
cd ../sketches-core-cpp
git archive --format zip --output ../sketches-postgres/$COREARCH master
cd ../sketches-postgres

unzip $PGARCH
COREDIR=$DST/sketches-core-cpp
mkdir $COREDIR
unzip $COREARCH -d $COREDIR

zip -r $DST.zip $DST
