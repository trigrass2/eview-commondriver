#!/bin/bash 当前目录需是setup/arm-linux
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

ftp -v -n ftp.peakinfo.cn 2121 <<EOF
user peak peak
binary
prompt
lcd ../../repo 
mkdir /release/eview-server/ubuntu14
mkdir /release/eview-server/ubuntu14/backup
cd /release/eview-server/ubuntu14
mput eview-server-commondriver.tar.gz 
cd /release/eview-server/arm-linux/backup 
mput eview-server-commondriver-*.tar.gz 
close
bye
!

cd ${CURDIR}
