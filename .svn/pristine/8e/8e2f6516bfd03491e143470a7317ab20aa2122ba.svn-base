#!/bin/bash
CURDIR=$(cd `dirname $0`; pwd)
cd ${CURDIR}

ftp -v -n 192.168.10.1 21 <<EOF
user peak peak 
binary 
 
prompt 
lcd ../../repo 
mkdir /release/eview-server/centos7.0
mkdir /release/eview-server/centos7.0/backup 
cd /release/eview-server/centos7.0 
mput eview-server-commondriver.tar.gz 

cd /release/eview-server/centos7.0/backup 
mput eview-server-commondriver-*.tar.gz 
close 
bye 
! 

cd ${CURDIR}
