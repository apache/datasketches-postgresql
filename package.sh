#!/bin/bash -e

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# This schipt is to prepare a package for pgxn.org

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
CORETAG=5.2.0

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

# extra readme files confuse pgxn.org
rm $COREDIR/README.md

zip -r $DST.zip $DST

rm -f $PGARCH $COREARCH
rm -rf $DST
