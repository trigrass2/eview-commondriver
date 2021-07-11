#####从ftp服务器上的/release/cccomps/arm-linux 到 本地的thirdparty 
#!/bin/bash
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}
rm -rf ../../thirdparty
mkdir ../../thirdparty

ftp -v -n 192.168.10.1 21 <<EOF
user peak peak
binary
passive
prompt
lcd ../../thirdparty
cd /release/cccomps/arm-linux
mget pkcomm.tar.gz pkdbapi.tar.gz pkmqtt.tar.gz jsoncpp.tar.gz pkce.tar.gz snmp_pp.tar.gz libdes.tar.gz

cd /release/eview-server/arm-linux
mget eview-server-driversdk.tar.gz
mget eview-pkdata.tar.gz
close
bye
!


