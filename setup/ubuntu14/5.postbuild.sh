#!/bin/sh
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

echo prepare eveiw-server directory
rm -rf ../../repo/eview-server
mkdir -p ../../repo/eview-server/bin/drivers

#sample driver
rm -rf ../../bin/drivers/samplepythondrv/ ../../bin/drivers/htmlpostdrv/ ../../bin/drivers/htmlquerydrv/
cp -rf ../../source/drivers/samplepythondrv ../../bin/drivers/samplepythondrv/
cp -rf ../../source/drivers/htmlpostdrv ../../bin/drivers/htmlpostdrv/
cp -rf ../../source/drivers/htmlquerydrv/ ../../bin/drivers/htmlquerydrv/

echo copy all bin with drivers
 cp -rf ../../bin/drivers ../../repo/eview-server/bin/

echo now package it ......

rm -f ../../repo/eview-server-commondriver*.tar.gz
mkdir -p ../../repo
tar -cvzf ../../repo/eview-server-commondriver.tar.gz -C ../../repo/eview-server/ . 
CURDATE=$(date +%Y%m%d)
cp  -f ../../repo/eview-server-commondriver.tar.gz ../../repo/eview-server-commondriver-${CURDATE}.tar.gz

cd ${CURDIR}
