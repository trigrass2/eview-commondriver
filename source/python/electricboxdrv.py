#! /usr/bin/python
# coding=utf-8

import sys
import struct
import os

python_file = __file__
bin_path = os.path.dirname(__file__) + '\\..\\..'
python_path = os.path.dirname(__file__) + '\\..\\..\\..\\python'
print(bin_path, python_path)
sys.path.append(bin_path)
sys.path.append(python_path)
print(sys.path)
import pydriver

def InitDriver(pkDriver):
    print("----InitDriver electricboxdrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver electricboxdrv----")
    print(pkDriver)
    return 0

def InitDevice(pkDevice):
    print("----start InitDevice " + pkDevice['name'] + "----")
    print(pkDevice)
    tags = []
    for tag in pkDevice['tags']:
        tags.append(tag)
    print("----end InitDevice" + pkDevice['name'] + "----")
     # timerObj=pydriver.CreateTimer(pkDevice, 3000, tags)
    return 0

def UnInitDevice(pkDevice):
    print("UnInitDevice" + pkDevice['name'])
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    print("----OnTimer----")
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")

    # 获取设备地址参数
    tagAddr = ""
    buffer = ['' * 8] * 4
    # 控制切换信号源
    if (pkTag['name'] == "electricbox.box_switch"):
        tagAddr = "box_switch"
        print("===recv switch command===")
        if (strTagValue == 0):
            buffer[0] = struct.pack("8B", 0x01, 0x05, 0x00, 0x10, 0xff, 0x00, 0x8d, 0xff)
            buffer[1] = struct.pack("8B", 0x01, 0x05, 0x00, 0x11, 0xff, 0x00, 0xdc, 0x3f)
            buffer[2] = struct.pack("8B", 0x01, 0x05, 0x00, 0x12, 0xff, 0x00, 0x2c, 0x3f)
            buffer[3] = struct.pack("8B", 0x01, 0x05, 0x00, 0x13, 0xff, 0x00, 0x7d, 0xff)
        elif(strTagValue == 1):
            buffer[0] = struct.pack("8B", 0x01, 0x05, 0x00, 0x10, 0x00, 0x00, 0xcc, 0x0f)
            buffer[1] = struct.pack("8B", 0x01, 0x05, 0x00, 0x11, 0x00, 0x00, 0x9d, 0xcf)
            buffer[2] = struct.pack("8B", 0x01, 0x05, 0x00, 0x12, 0x00, 0x00, 0x6d, 0xcf)
            buffer[3] = struct.pack("8B", 0x01, 0x05, 0x00, 0x13, 0x00, 0x00, 0x3c, 0x0f)
        else:
            print("===invalid command===")
            return
        print(buffer)

    # 向设备发送指令 返回成功发送的字节数
    result = pydriver.Send(pkDevice, buffer, 2000)
    if (result <= 0):                            # 发送数据失败 通讯线未插好
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "box_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 接收设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("==recv:" + str(bufferRecv) + "==")
    print("==Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) +"==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "box_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 更新控制点的值
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, strTagValue)
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(strTagValue) + "==")

    # 更新通信状态
    tagValue = 0
    tagAddr = "box_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
    return 0