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

# # FA 0B 0B 01 01 01 01 01 01 01 01 01 09
# def findOnePakage(buffer):
#     for i in range(0, len(buffer)):
#         if (len(buffer) - i < 13):
#             return ""
#         if (buffer[i] != 0xfa)

def InitDriver(pkDriver):
    print("----InitDriver AvtronsysMatrixdrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver AvtronsysMatrixdrv----")
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

    # 向设备发送查询指令
    buffer = struct.pack("4B", 0x0c, 0x01, 0x01, 0x02)
    result = pydriver.Send(pkDevice, buffer, 2000)
    if (result <= 0):                            # 发送数据失败
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "matrix_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 接收设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("recv:", bufferRecv)
    print("Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "matrix_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 解析设备返回值
    tagValue = ""
    for i in range(10):
        tagValue += str(ord(bufferRecv[i+3]))
        tagValue += ","

    # 更新点状态
    tagAddr = "matrix_output"
    print(tagAddr)
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice,tagList, tagValue)
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")

    # 更新通信状态
    tagValue = 0
    tagAddr = "matrix_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")

    print("-OnTimer- end")
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")
    print(pkDevice, pkTag)
    # print("device name:", pkDevice['name'], "tagname:", pkTag['name'], "tagaddress:", pkTag['address'], "tagvalue:", strTagValue)

    # 获取设备地址参数
    devAddr = int(pkDevice['param2'])
    print ("==get device address " + str(devAddr) + "==")
    tagAddr = ""

    # 控制切换信号源
    if (pkTag['name'] == "avtronsysmatrix.matrix_switch"):
        tagAddr = "matrix_switch"
        print("recv switch command")
        input = strTagValue.split(",")[0]
        print("in:", input)
        output = strTagValue.split(",")[1]
        print("out:", output)
        buffer = struct.pack("4B", 0x0C, devAddr, int(input), int(output))
        print(buffer)

    # 向设备发送指令 返回成功发送的字节数
    result = pydriver.Send(pkDevice, buffer, 2000)
    if (result <= 0):                            # 发送数据失败 通讯线未插好
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "matrix_status"
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
        tagAddr = "matrix_status"
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
    tagAddr = "matrix_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
    return 0