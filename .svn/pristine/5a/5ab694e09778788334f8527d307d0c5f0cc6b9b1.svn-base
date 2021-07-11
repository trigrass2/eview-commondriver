#!/usr/bin/python
# -*- coding: gb18030 -*-

from opcdrv import *
from globalvars import *
#c++ 提供我这些方法：
def UpdateTagsData(hDevice, vecTags,nTagsSize, nStatus,timestamp):
     print 'c++：updatetagsdata'
def UpdateTagsData(hDevice, vecTags, nStatus):
   # UpdateTagsData(hDevice, vecTags,len(vecTags),nStatus,0);
    print 'updatetagsdata',vecTags

def CreateTimer(hDevice, timerInfo):
    global g_timers
    print "create timer success"
    g_timers.append((hDevice,timerInfo))

if __name__ == '__main__':
    global g_timers
    device = {}
    device['name']='maxtricon opc'
    device['desc']=''
    device['conntype']='openopc' #dcom
    device['connparams']='ip=192.168.10.108;server=Matrikon.OPC.Simulation.1'
    device['syncmode']=0
    device['pvdata']=None
    device['pvdata2']=None
    device['d_bConnected']=False
    device['tags']=[]

    device['tags'].append({'name':'opctag1','address':'Random.Int4','datatype1':'int32','pollrate':1000,'lenbits':0,'value':[],'datatype2':'short','datatype3':'long','quality':[],'timestamp':[],'datalen':4})
    device['tags'].append({'name':'opctag2','address':'Random.Real4','datatype1':'float','pollrate':500,'lenbits':0,'value':[],'datatype2':'short','quality':[],'datatype3':'long','timestamp':[],'datalen':4})
    device['tags'].append({'name':'opctag3','address':'Random.Real8','datatype1':'double','pollrate':500,'lenbits':0,'value':[],'datatype2':'short','quality':[],'datatype3':'long','timestamp':[],'datalen':8})

    device2 = {}
    device2['name']='maxtricon opc'
    device2['desc']=''
    device2['conntype']='openopc' #dcom
    device2['connparams']='ip=192.168.10.115;server=Matrikon.OPC.Simulation.1'
    device2['syncmode']=0
    device2['pvdata']=None
    device2['pvdata2']=None
    device2['d_bConnected']=False
    device2['tags']=[]

    device2['tags'].append({'name':'opctag1','address':'Random.Int4','datatype1':'int32','pollrate':1000,'lenbits':0,'value':[],'datatype2':'short','datatype3':'long','quality':[],'timestamp':[],'datalen':4})
    device2['tags'].append({'name':'opctag2','address':'Random.Real4','datatype1':'float','pollrate':1000,'lenbits':0,'value':[],'datatype2':'short','quality':[],'datatype3':'long','timestamp':[],'datalen':4})
    device2['tags'].append({'name':'opctag3','address':'Random.Real8','datatype1':'double','pollrate':500,'lenbits':0,'value':[],'datatype2':'short','quality':[],'datatype3':'long','timestamp':[],'datalen':8})

    InitDevice(device)
    InitDevice(device2)
    while(True):
        for (hDevice,timerInfo) in g_timers:
            OnTimer(hDevice, timerInfo)
        #写值
        #tag =  {'name':'opctag3','address':'Triangle Waves.Int1','datatype1':'double','pollrate':500,'lenbits':0,'value':[],'datatype2':'short','quality':[],'datatype3':'long','timestamp':[],'datalen':8}
        #OnControl(device, tag, 100,0)
        time.sleep(1)