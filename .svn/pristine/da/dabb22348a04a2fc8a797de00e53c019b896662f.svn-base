#!/bin/bash
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

ftp -v -n 192.168.10.1 21 <<EOF
user peak peak 
binary 
passive 
prompt 
lcd ../../repo 
mkdir /release/eview-server/arm-linux
mkdir /release/eview-server/arm-linux/backup 
cd /release/eview-server/arm-linux 
mput eview-server-commondriver.tar.gz 

cd /release/eview-server/arm-linux/backup 
mput eview-server-commondriver-*.tar.gz 
close 
bye 
! 

cd ${CURDIR}
