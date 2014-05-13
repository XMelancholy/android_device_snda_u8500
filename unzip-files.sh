#/bin/sh

VENDOR=snda
DEVICE=u8500

UPDATE_PACKAGE=~/vendor/snda/snda4.1.zip

BASE=../../../vendor/$VENDOR/$DEVICE/proprietary
rm -rf $BASE/*

for FILE in `egrep -v '(^#|^$)' proprietary-files.txt`; do
  DIR=`dirname $FILE`
  if [ ! -d $BASE/$DIR ]; then
    mkdir -p $BASE/$DIR
  fi
  unzip -p ${UPDATE_PACKAGE} system/$FILE > $BASE/$FILE
done

./setup-makefiles.sh
