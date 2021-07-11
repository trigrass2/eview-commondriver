#!/bin/sh
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

echo extract depends component......
rm -rf ../../bin/
rm -rf ../../lib/
mkdir ../../bin ../../lib ../../include
#extract
tar -xvzf ../../thirdparty/pkcomm.tar.gz -C ../../
tar -xvzf ../../thirdparty/pkmqtt.tar.gz -C ../../
tar -xvzf ../../thirdparty/jsoncpp.tar.gz -C ../../
tar -xvzf ../../thirdparty/pkdbapi.tar.gz -C ../../
tar -xvzf ../../thirdparty/pkce.tar.gz -C ../../
tar -xvzf ../../thirdparty/snmp_pp.tar.gz -C ../../
tar -xvzf ../../thirdparty/libdes.tar.gz -C ../../
tar -xvzf ../../thirdparty/eview-server-driversdk.tar.gz -C ../../
tar -xvzf ../../thirdparty/eview-pkdata.tar.gz -C ../../

cd ${CURDIR}
