#!/bin/bash

VER=$1

DST=datasketches-$VER

PGARCH=postgresql-$VER.zip
COREARCH=core-$VER.zip

rm -rf $DST
rm $DST.zip

git archive --format zip --prefix=$DST/ --output $PGARCH master
cd ../datasketches-cpp
git archive --format zip --output ../datasketches-postgresql/$COREARCH master
cd ../datasketches-postgresql

unzip $PGARCH
COREDIR=$DST/datasketches-cpp
mkdir $COREDIR
unzip $COREARCH -d $COREDIR

zip -r $DST.zip $DST
