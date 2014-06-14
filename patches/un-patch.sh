#!/bin/sh
#
# applypatch.sh
# un apply patches for snda_s1
#
# Author: XianGxin
# Create: 2014-06-14 05:06


dir=`cd $(dirname $0) && pwd`
top=$dir/../../../..

for patch in `ls $dir/*.patch` ; do
	patch -p1 -R -N -i $patch -r - -d $top
done

