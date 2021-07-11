#####从ftp服务器上的/release/cccomps/arm-linux 到 本地的thirdparty 
#!/bin/bash
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

mkdir ../../thirdparty

ftp -v -n ftp.peakinfo.cn 2121 <<EOF
user peak peak
binary
prompt

lcd ../../thirdparty
cd /release/cccomps/ubuntu14
mget pkcomm.tar.gz pkdbapi.tar.gz pkmqtt.tar.gz jsoncpp.tar.gz pkce.tar.gz snmp_pp.tar.gz libdes.tar.gz

cd /release/eview-server/ubuntu14
mget eview-server-driversdk.tar.gz
mget eview-pkdata.tar.gz

close
bye
!
cd ${CURDIR}
