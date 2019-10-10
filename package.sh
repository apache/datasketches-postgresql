#!/bin/bash -e

VER=$1
if [ -z $1 ]; then
  echo target PGXN package version must be specified: x.y.z
  exit;
fi

TAG=$2
if [ -z $2 ]; then
  echo datasketches-postgresql git tag must be specified
  exit;
fi

# version of datasketches-cpp core library to include
CORETAG=1.0.0-incubating

DST=datasketches-$VER

PGARCH=postgresql-$VER.zip
COREARCH=core-$CORETAG.zip

rm -rf $DST
rm -f $DST.zip

git archive --format zip --prefix=$DST/ --output $PGARCH $TAG
cd ../datasketches-cpp
git archive --format zip --output ../datasketches-postgresql/$COREARCH $CORETAG
cd ../datasketches-postgresql

unzip $PGARCH
COREDIR=$DST/datasketches-cpp
mkdir $COREDIR
unzip $COREARCH -d $COREDIR

zip -r $DST.zip $DST

rm $PGARCH $COREARCH
rm -r $DST
