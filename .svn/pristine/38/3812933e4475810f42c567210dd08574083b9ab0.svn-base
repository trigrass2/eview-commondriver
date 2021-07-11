#! /usr/bin/python
# coding=utf-8

import sys
import struct
import os

python_file=__file__
bin_path=os.path.dirname(__file__)+ '\\..\\..'
python_path=os.path.dirname(__file__)+'\\..\\..\\..\\python'
print(bin_path,python_path)
sys.path.append(bin_path)
sys.path.append(python_path)
print(sys.path)
import pydriver

def InitDriver(pkDriver):
    print("----InitDriver NightSunStageLampdrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver NightSunStageLampdrv----")
    print(pkDriver)
    return 0

def InitDevice(pkDevice):
    print("----start InitDevice " + pkDevice['name'] + "----")
    print(pkDevice)
    tags=[]
    for tag in pkDevice['tags']:
        tags.append(tag)
    print("----end InitDevice" + pkDevice['name'] + "----")
    return 0

def UnInitDevice(pkDevice):
    print("==UnInitDevice" + str(pkDevice['name']) + "==")
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    return 0


def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")
    print(pkDevice,pkTag)

    tagAddr = ""
    # 组织控制命令
    # 控制舞台灯光开关
    if (pkTag['name'] == "nightsunstagelamp.stage_switch"):
        tagAddr = "stage_switch"
        # 关闭电源
        if (strTagValue == "0"):
            buffer = struct.pack("B", 0x04)
        # 开启电源
        elif (strTagValue == "1"):
            buffer = struct.pack("B", 0x09)

    # 控制切换灯光效果
    if (pkTag['name'] == "nightsunstagelamp.stage_apply"):
        tagAddr = "stage_apply"
        # 场景1（柔和）
        if (strTagValue == "1"):
            buffer = struct.pack("B", 0x01)
        # 场景2（明亮）
        elif (strTagValue == "2"):
            buffer = struct.pack("B", 0x02)
        # 场景3（幻彩）
        elif (strTagValue == "3"):
            buffer = struct.pack("B", 0x03)
        # 场景4（娱乐）
        elif (strTagValue == "4"):
            buffer = struct.pack("B", 0x05)
        # 灯光效果6
        elif (strTagValue == "5"):
            buffer = struct.pack("B", 0x06)
        # 灯光效果7
        elif (strTagValue == "6"):
            buffer = struct.pack("B", 0x07)
        # 灯光效果8
        elif (strTagValue == "7"):
            buffer = struct.pack("B", 0x08)
        # 灯光效果9
        elif (strTagValue == "8"):
            buffer = struct.pack("B", 0x0a)
        # 灯光效果10
        elif (strTagValue == "9"):
            buffer = struct.pack("B", 0x0b)
        # 灯光效果11
        elif (strTagValue == "10"):
            buffer = struct.pack("B", 0x0c)
        # 灯光效果12
        elif (strTagValue == "11"):
            buffer = struct.pack("B", 0x0d)
        # 灯光效果13
        elif (strTagValue == "12"):
            buffer = struct.pack("B", 0x0e)

    # 发送控制命令
    result = pydriver.Send(pkDevice, buffer, 2000)
    if (result <= 0):  # 发送数据失败 通讯线未插好
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "stage_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 更新控制点
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(strTagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(strTagValue) + "==")

    # 接收设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("recv:", bufferRecv)
    print("==Recv,retcode:" + str(retCode), ",length:"+ str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "stage_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue))
        return

    # 更新控制点
    # tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    # pydriver.SetTagsValue(pkDevice, tagList, str(strTagValue))
    # pydriver.UpdateTagsData(pkDevice, tagList)
    # for i in range(0, len(tagList)):
    #     print("==update tag:"+str(tagList[i]['name'])+" value:" + str(strTagValue) + "==")

    # 更新通信状态
    tagValue = 0
    tagAddr = "stage_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")

    return 0