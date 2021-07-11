#!/usr/bin/python
# -*- coding: gb18030 -*-

'''
Created on 2014-9-24

@author: shijunpu
hostname  存放在当前目录的hostname文件下
hostid    存放在当前目录的hostid目录下
如果设备网卡未设置网关或未正确设置网关，那么设备无法发出udp应答包（sendto: Network is unreachable），但这时候实际上还是可以接收包的，故如果知道hostid则可以继续设置网关
'''

import socket
import time
import struct
import uuid
import os
import json
import string
import re
import platform
import datetime
import time
import  OpenOPC
from main import *

g_lastConnTime=time.time()


def Tags2Group(syncmode,tags):
    groups = []
    for tag in tags:
        bTagAdded = False
        for group in groups:
            groupRate = group.get('pollrate',1000)
            if(tag.get('pollrate',1000) != groupRate):
                continue

            #相同的rate
            group['tags'].append(tag)
            bTagAdded = True
            break

        if(bTagAdded == False):
            #还没有合适的Group
            group = {}
            group['pollrate']=tag.get('pollrate',1000)
            group['syncmode']=syncmode
            group['name']="group_%d"%(len(groups)+1)
            group['tags']=[tag]
            groups.append(group)
    return groups

def ValueDataToGroupsData(tags,values):
     for tag in tags:
         for data in values:
           (address,value,quality,timestamp)=data
           if(tag['address']==address):
               if(type(value)==int):
                   tag["value"] = value
               #质量好的为1，坏的为0,为止和不确定为2
               if(quality=='Good'):
                   tag["quality"] = 1
               else :
                   if(quality=='Bad'):
                        tag["quality"] = 0
                   else:
                       tag["quality"] = 2
               if(type(timestamp)==str):
                    #str ='10/16/15 13:57:06'转datetime
                    # ltimestamp = datetime.datetime.strptime(timestamp,'%m/%d/%Y %H:%M:%S')
                    tag["timestamp"] = timestamp



def InitDevice(device):
    global g_timers
    print 'InitDevice'
    tags=device.get('tags',[])
    syncmode=device['syncmode']
    groups = Tags2Group(syncmode,tags)
    device['groups'] = groups
    for group in groups:
        timerInfo={}
        timerInfo["group"] = group
        timerInfo["pollrate"]  = group['pollrate']
        CreateTimer(device,timerInfo)

    tryConnToRemoteOPC(device)

def tryConnToRemoteOPC(device,group):
    d_bConnected  =  device['d_bConnected']

    if(d_bConnected == True):
        return d_bConnected

    if(time.time() - g_lastConnTime <3):
        d_bConnected = False
        device['d_bConnected'] = d_bConnected
        return d_bConnected;

    #获取IP 和服务名称IP='127.';server=''
    connIPAndName = device.get('connparams','ip=127.0.0.1;server=Matrikon.OPC.Simulation.1')
    ip = connIPAndName[connIPAndName.find('ip=')+len('ip='):connIPAndName.find(';')]
    #ip = '192.168.10.108'
    #server= 'Matrikon.OPC.Simulation.1'
    server = connIPAndName[connIPAndName.find('server=')+len('server='):len(connIPAndName)]

    try:
        opc=OpenOPC.open_client(ip)
        device['pvdata']=opc

    except Exception as e:
        print "Error Massage: ",e.message
        print "conn to opc failed on ",ip
        d_bConnected = False
        device['d_bConnected'] = d_bConnected
        return d_bConnected

    try:
        opc.connect(server)

    except Exception as e:
         print "Error Massage: ",e.message
         print "conn to opc failed,The ERROR server is ",server
         d_bConnected = False
         device['d_bConnected'] = d_bConnected
         return d_bConnected
    #删除组
    try:
        opc.remove(opc.groups())
    except Exception as e:
         print "Error Massage: ",e.message
         print "group remove failed at The server : ",server
         d_bConnected = False
         device['d_bConnected'] = d_bConnected
         return d_bConnected

     #新建读取组

    goupItemID = []

    for group in device['groups']:
           for tag in group:
              goupItemID.append(tag['address'])
           try:
              opc.read(goupItemID, group=group['name'])
           except Exception as e:
                 print "Error Massage: ",e.message
                 print "group add failed at The server : ",server
                 d_bConnected = False
                 device['d_bConnected'] = d_bConnected
                 return d_bConnected

    d_bConnected = True
    if(d_bConnected == True):
          print "conn to opc success,The Server is",server
          device['d_bConnected'] = d_bConnected
    return d_bConnected

def OnTimer(device, timerInfo):
    group = timerInfo['group']
    is_bConnected = tryConnToRemoteOPC(device,group)
    if(is_bConnected == False):
        return 0

    opc = device['pvdata']

    goupItemID = []
    for tag in group['tags']:
        goupItemID.append(tag['address'])
    goupItemID.append('erro')
    data = []
    try:
         data = opc.read(goupItemID )
    except Exception as e:
        device['d_bConnected'] = False
        print "read null",e.message
        return  0
    print "data:",data
    ValueDataToGroupsData(group['tags'],data)
    UpdateTagsData(device, group['tags'], 0)


def OnControl(device, tag, value,lCmdId):
    is_bConnected = False
    is_bConnected = tryConnToRemoteOPC(device)
    if(is_bConnected == False):
        return 0
    opc = device['pvdata']
    w_Result = opc.write( (tag['address'],value) )
    print "Write The Value Into Server",w_Result
    opc.close()
    return w_Result=='Success'#True表示写值成功，False写值失败