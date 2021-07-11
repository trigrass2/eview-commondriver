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
import logging
import pydriver

def InitDriver(pkDriver):
    print("----InitDriver HoneyWellAudiodrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver HoneyWellAudiodrv----")
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
    print("==UnInitDevice " + str(pkDevice['name']) + "==")
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    logging.basicConfig(level=logging.DEBUG,filename='/new.log',filemode='a',format='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s')
    logging.info('====OnControl!!!====')
    print("==pylog:-!--OnControl--!==")
    print(pkDevice,pkTag)

    # 组织控制命令
    # 控制总音量变化
    tagAddr = ""
    buffer = ['' * 8] * 8
    if (pkTag['name'] == "honeywellaudio.vol_change"):
        tagAddr = "vol_change"
        # 增大音量
        if (strTagValue == "1"):
            # 读取音量值
            buffer[0] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x01, 0x01, 0x0A, 0x41)
            buffer[1] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x02, 0x02, 0x0A, 0x43)
            buffer[2] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x03, 0x03, 0x0A, 0x45)
            buffer[3] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x04, 0x04, 0x0A, 0x47)
            buffer[4] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x05, 0x05, 0x0A, 0x49)
            buffer[5] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x06, 0x06, 0x0A, 0x4B)
            buffer[6] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x07, 0x07, 0x0A, 0x4D)
            buffer[7] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x33, 0x08, 0x08, 0x0A, 0x4F)
        # 减小音量
        if (strTagValue == "0"):
            buffer[0] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x01, 0x01, 0x0A, 0x42)
            buffer[1] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x02, 0x02, 0x0A, 0x44)
            buffer[2] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x03, 0x03, 0x0A, 0x46)
            buffer[3] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x04, 0x04, 0x0A, 0x48)
            buffer[4] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x05, 0x05, 0x0A, 0x4A)
            buffer[5] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x06, 0x06, 0x0A, 0x4C)
            buffer[6] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x07, 0x07, 0x0A, 0x4E)
            buffer[7] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x34, 0x08, 0x08, 0x0A, 0x50)
    # 控制静音和取消
    if (pkTag['name'] == "honeywellaudio.vol_mute"):
        tagAddr = "vol_mute"
        # 取消静音
        if (strTagValue == "1"):
            buffer[0] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x01, 0x01, 0x00, 0x36)
            buffer[1] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x02, 0x02, 0x00, 0x38)
            buffer[2] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x03, 0x03, 0x00, 0x3A)
            buffer[3] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x04, 0x04, 0x00, 0x3C)
            buffer[4] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x05, 0x05, 0x00, 0x3E)
            buffer[5] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x06, 0x06, 0x00, 0x40)
            buffer[6] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x07, 0x07, 0x00, 0x42)
            buffer[7] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x32, 0x08, 0x08, 0x00, 0x44)
        # 静音
        if (strTagValue == "0"):
            buffer[0] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x01, 0x01, 0x00, 0x35)
            buffer[1] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x02, 0x02, 0x00, 0x37)
            buffer[2] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x03, 0x03, 0x00, 0x39)
            buffer[3] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x04, 0x04, 0x00, 0x3B)
            buffer[4] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x05, 0x05, 0x00, 0x3D)
            buffer[5] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x06, 0x06, 0x00, 0x3F)
            buffer[6] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x07, 0x07, 0x00, 0x41)
            buffer[7] = struct.pack("8B", 0xA5, 0xAB, 0x02, 0x31, 0x08, 0x08, 0x00, 0x43)

    logging.info('====recv cmd succ!!!====')
    # 发送控制命令
    for i in range(8):
        result = pydriver.Send(pkDevice, buffer[i], 2000)
        print("==SendToDevice:" + str(result) + "==")
        if (result <= 0):  # 发送数据失败 通讯线未插好
            print("==send to device fail==")
            print("==send to device:", len(buffer), "return :" + str(result) + "==")
            tagValue = -1
            tagAddr = "vol_status"
            tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
            pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
            pydriver.UpdateTagsData(pkDevice, tagList)
            print("==pylog:fail to send data==")
            for i in range(0, len(tagList)):
                print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
            return -1

    logging.info('====send buff succ!!!====')
    # 接收设备返回
    retCode, bufferRecv = pydriver.Recv(pkDevice, 10000, 200)
    print("==recv:" + str(bufferRecv) + "==")
    print("==Recv,retcode:" + str(retCode) + ",length:" + str(len(bufferRecv)) + "==")
    if(retCode != 0):                           # 未获取到设备返回值 通信故障
        tagValue = -2
        tagAddr = "vol_status"
        tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
        pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
        pydriver.UpdateTagsData(pkDevice, tagList)
        print("==pylog:send to device return <-0==")
        for i in range(0, len(tagList)):
            print("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
            logging.info("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")
        return -1

    # 更新控制点状态
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, strTagValue)
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:" + str(tagList[i]['name']) + " value:" + str(strTagValue) + "==")
        logging.info("==update tag:" + str(tagList[i]['name']) + " value:" + str(strTagValue) + "==")

    # 更新通信状态点
    tagValue = 0
    tagAddr = "vol_status"
    tagList = pydriver.GetTagsByAddr(pkDevice, tagAddr)
    pydriver.SetTagsValue(pkDevice, tagList, str(tagValue))
    pydriver.UpdateTagsData(pkDevice, tagList)
    for i in range(0, len(tagList)):
        print("==update tag:"+str(tagList[i]['name'])+" value:" + str(tagValue) + "==")
        logging.info("==update tag:" + str(tagList[i]['name']) + " value:" + str(tagValue) + "==")

    return 0