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
    print("----InitDriver honeywellpowersupplydrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver honeywellpowersupplydrv----")
    print(pkDriver)
    return 0

def InitDevice(pkDevice):
    print("----start InitDevice " + pkDevice['name'] + "----")
    print(pkDevice)
    tags=[]
    for tag in pkDevice['tags']:
        tags.append(tag)
    print("----end InitDevice" + pkDevice['name'] + "----")
    timerObj=pydriver.CreateTimer(pkDevice, 3000, tags)
    return 0

def UnInitDevice(pkDevice):
    print("UnInitDevice")
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    print("----OnTimer----")
    tags = pkTimerParam
    print(timerObj)
    print(tags)
    # 获得设备地址参数 组织报文
    devAddr = int(pkDevice['param2'])
    buffer = struct.pack("6B", 0x55, devAddr, 0x00, 0x00, 0xfa, 0xaa)
    # 向设备发送查询指令
    result = pydriver.Send(pkDevice, buffer, 2000)
    print("==SendToDevice:" + str(result) + "==")
    if (result <= 0):                            # 发送数据失败
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "power_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return
    # 获取设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("==Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "power_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return
    # 解析并更新点状态
    bufferRecv = ord(bufferRecv[2])
    for i in range(8):
        tagValue = (bufferRecv % pow(2, i+1)) / pow(2, i)               # 取bufferRecv的第i位
        tagAddr = "power_output" + str(i+1)
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice,tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
    # 更新通信状态
    tagValue = 0
    tagAddr = "power_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
    print("==OnTimer end==")
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")
    print(pkDevice,pkTag)

    # 组织控制命令
    # 控制总电源开关
    devAddr = int(pkDevice['param2'])
    if (pkTag['name'] == "honeywellpowersupply.power_switch"):
        # 关闭总电源
        if (strTagValue == "0"):
            buffer = struct.pack("6B", 0x55, devAddr, 0x00, 0x0d, 0xF1, 0xAA)
        # 开启总电源
        elif (strTagValue == "1"):
            buffer = struct.pack("6B", 0x55, devAddr, 0x00, 0x0d, 0xF0, 0xAA)
        tagAddr = "power_switch"
    # 控制单路电源切换
    if (pkTag['name'] == "honeywellpowersupply.power_switch_output"):
        Ch_ID = strTagValue.split(",")[0]
        if strTagValue.split(",")[1] == '0':
            cmd = 0xF1
        else:
            cmd = 0xF2
        buffer = struct.pack("6B", 0x55, devAddr, 0x00, int(Ch_ID), cmd, 0xAA)
        tagAddr = "power_switch_output"

    # 发送控制命令
    result = pydriver.Send(pkDevice, buffer, 2000)
    print("SendToDevice:" + str(result))
    if (result <= 0):                                    # 发送数据失败
        print("send to device fail")
        print("send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "power_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 获取设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "power_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 更新控制点状态
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, strTagValue)
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(strTagValue) + "==")

    # 更新通信状态


    tagValue = 0
    tagAddr = "power_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")

    return 0