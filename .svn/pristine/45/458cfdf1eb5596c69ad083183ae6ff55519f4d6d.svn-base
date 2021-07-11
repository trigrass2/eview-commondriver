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
    # timerObj=pydriver.CreateTimer(pkDevice, 3000, tags)
    return 0

def UnInitDevice(pkDevice):
    print("==UnInitDevice" + pkDevice['name'] + "==")
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print("-!--OnControl--!-")
    print(pkDevice,pkTag)
    # print("device name:",pkDevice['name'], "tagname:", pkTag['name'], "tagaddress:",pkTag['address'], "tagvalue:",strTagValue)

    # 组织控制命令
    # 发送红外控制命令
    if (pkTag['name'] == "infrared.dvd_on"):
        tagAddr = "dvd_on"
        # 开启影碟机电源
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x01, 0x00, 0x50)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_off"):
        tagAddr = "dvd_off"
        # 关闭影碟机电源
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x02, 0x00, 0x53)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_open"):
        tagAddr = "dvd_open"
        # 影碟机开仓
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x03, 0x00, 0x52)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_close"):
        tagAddr = "dvd_close"
        # 影碟机关仓
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x04, 0x00, 0x55)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_prev"):
        tagAddr = "dvd_prev"
        # 影碟机上一首
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x05, 0x00, 0x54)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_next"):
        tagAddr = "dvd_next"
        # 影碟机下一首
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x06, 0x00, 0x57)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_play"):
        tagAddr = "dvd_play"
        # 影碟机播放
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x07, 0x00, 0x56)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_pause"):
        tagAddr = "dvd_pause"
        # 影碟机暂停
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x08, 0x00, 0x59)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.dvd_stop"):
        tagAddr = "dvd_stop"
        # 影碟机停止
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x51, 0x09, 0x00, 0x58)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv1_on"):
        tagAddr = "tv1_on"
        # 电视机1开启
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x52, 0x0a, 0x00, 0x58)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv1_off"):
        tagAddr = "tv1_off"
        # 电视机1关闭
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x52, 0x0b, 0x00, 0x59)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv2_on"):
        tagAddr = "tv2_on"
        # 电视机2开启
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x53, 0x0a, 0x00, 0x59)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv2_off"):
        tagAddr = "tv2_off"
        # 电视机2关闭
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x53, 0x0b, 0x00, 0x58)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv3_on"):
        tagAddr = "tv3_on"
        # 电视机3开启
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x54, 0x0a, 0x00, 0x5e)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv3_off"):
        tagAddr = "tv3_off"
        # 电视机3关闭
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x54, 0x0b, 0x00, 0x5f)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv4_on"):
        tagAddr = "tv4_on"
        # 电视机4开启
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x55, 0x0a, 0x00, 0x5f)
        else:
            print ("==invalid command!==")
            return
    elif (pkTag['name'] == "infrared.tv4_off"):
        tagAddr = "tv4_off"
        # 电视机4关闭
        if (strTagValue == "1"):
            buffer = struct.pack("6B", 0x50, 0xfa, 0x55, 0x0b, 0x00, 0x5e)
        else:
            print ("==invalid command!==")
            return

    # 发送控制命令
    result = pydriver.Send(pkDevice, buffer, 2000)
    if (result <= 0):  # 发送数据失败
        print("==send to device fail==")
        print("==send to device:" + str(len(buffer)) + "return :" + str(result) + "==")
        tagValue = -1
        tagAddr = "infrared_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 获取设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("Recv,retcode:", retCode, ",length:", len(bufferRecv))
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "infrared_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("send to device return <-0")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return

    # 更新控制点
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(strTagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(strTagValue) + "==")

    # 更新通信状态
    tagValue = 0
    tagAddr = "infrared_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")

    return 0