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
    print("----InitDriver stitchingdisplaydrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver stitchingdisplaydrv----")
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

    # 发送查询指令
    cmd = "<RPWR>"
    buffer = struct.pack("!9s", cmd)
    result = pydriver.Send(pkDevice, buffer, 2000)
    print("==SendToDevice:" + str(result) + "==")
    if (result <= 0):                            # 发送数据失败
        print("==send to device fail==")
        print("==send to device:"+ str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "screen_mul_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, tagValue)
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + tagValue + "==")
        return

    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("==Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "screen_mul_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, tagValue)
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + tagValue + "==")
        return

    # 解析数据 得到值
    tagValue =struct.unpack("!"+ str(len(bufferRecv)) + "s", bufferRecv)
    print("==recv tagValue:" + str(tagValue[0]) + "==")
    # 电源关闭
    if (tagValue[0] == "<RPWR,0>"):
        tagValue = 0
    elif (tagValue[0] == "<RPWR,1>"):
        tagValue = 1
    elif (tagValue[0] == "<RPWR,RTER>"):
        tagValue = 2

    # 更新电源状态
    tagAddr = "screen_mul_power"
    print(tagAddr)
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice,tagList, str(tagValue[0]))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue[0]) + "=")

    # 更新通信状态
    tagValue = 0
    tagAddr = "screen_mul_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")

    print("-OnTimer- end")
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")
    print(pkDevice,pkTag)
    # print("device name:",pkDevice['name'], "tagname:", pkTag['name'], "tagaddress:",pkTag['address'], "tagvalue:",strTagValue)

    # 控制屏幕开关
    if (pkTag['name'] == "stitchingdisplayscreen.screen_mul_switch"):
        tagAddr = "screen_mul_switch"
        # 关闭电源
        if (strTagValue == "0"):
            cmd = "<WPWR,0>"
        # 开启电源
        elif (strTagValue == "1"):
            cmd = "<WPWR,1>"

    # 控制单屏切换信号源
    if (pkTag['name'] == "stitchingdisplayscreen.screen_mul_change"):
        tagAddr = "screen_mul_change"
        Wnd_ID = strTagValue.split(",")[0]
        Sig_ID = strTagValue.split(",")[1]
        cmd = "< MOVS, " + Wnd_ID + ", " + Sig_ID + " >"

    # 切换屏幕布局
    if (pkTag['name'] == "stitchingdisplayscreen.screen_mul_apply"):
        tagAddr = "screen_mul_apply"
        cmd = "<CALL" + str(strTagValue) + ">"

    # 发送控制命令
    buffer = struct.pack("!" + str(len(cmd)+1) + "s", cmd)
    result = pydriver.Send(pkDevice, buffer, 2000)
    print("==SendToDevice:" + str(result) + "==")
    if (result <= 0):  # 发送数据失败 通讯线未插好
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result))
        tagValue = -1
        tagAddr = "screen_mul_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 获取设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("==Recv,retcode:" +  str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "screen_mul_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 更新控制点
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice,tagList, str(strTagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(strTagValue) + "==")

    # 更新通信状态
    tagValue = 0
    tagAddr = "screen_mul_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")

    return 0
