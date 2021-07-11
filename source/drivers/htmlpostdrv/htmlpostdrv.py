#! /usr/bin/python
# coding=utf-8

import sys
import os
import struct
import urllib2
import requests
import json
import urllib

sys.path.append(os.path.abspath('.') + '\\..\\..')
sys.path.append(os.path.abspath('.') + '\\..\\..\\python')
import sys
reload(sys)
sys.setdefaultencoding('gbk')

import pydriver
import login
from pyquery import PyQuery as pq

def InitDriver(pkDriver):
    print "----InitDriver----"
    #print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print "----UnInitDriver----"
    #print(pkDriver)
    return 0

def InitDevice(pkDevice):
    print "----InitDevice----"
    print(pkDevice)
    #根据tag地址中分号前的网页url部分分组
    tags = []
    for tag in pkDevice['tags']:
        address = tag["address"]#POSTURL;PARAM
        tags.append(tag)
    timerObj = pydriver.CreateTimer(pkDevice, 3000, tags)

    print "device:", pkDevice["name"], "tagcount:", len(tags)

    session = requests.Session()
    pkDevice["session"] = session

    connparam=pkDevice["connparam"]
    loginParam = pkDevice['param1']
    loginResHope = pkDevice['param2']

    print("device",pkDevice["name"],"connparam:", connparam, "loginParam:", loginParam, "loginResHope:", loginResHope)
    loginArr = loginParam.split(';',1)
    loginUrl = loginArr[0]
    loginParamStr = ""
    if(len(loginArr) > 1):
        loginParamStr = loginArr[1].strip()

    #param1以逗号隔开
    loginUrlStr = pkDevice["connparam"] + loginUrl
    print(loginUrlStr)

    if(loginUrl is not None and loginUrl != ""): #需要登录
        loginResult = login.login_post(pkDevice, loginUrlStr, loginParamStr, loginResHope)
    else:
        loginResult = True

    pkDevice["loginResult"] = loginResult #保存登录结果
    print "device:", pkDevice["name"],",loginResult:", loginResult

    return 0

def UnInitDevice(pkDevice):
    print "UnInitDevice"
    return 0

#pkTimerParam=tags
def OnTimer(pkDevice, timerObj, pkTimerParam):
    print "----OnTimer----"
    tags=pkTimerParam
    session = pkDevice["session"]
    connparam=pkDevice["connparam"]

    for tag in tags:
        address = tag['address']
        addrArr = address.split(';',1)
        tagUrl = addrArr[0]
        postParamStr = ""
        if(len(addrArr) > 1):
            postParamStr = addrArr[1]

        jsonParam = {}
        if (postParamStr is None or postParamStr == ''):
            jsonParam = {}
        else:
            #postParamStr = urllib.quote(postParamStr)
            jsonParam = json.loads(postParamStr)

        postUrlStr = connparam + tagUrl
        session = pkDevice["session"]
        #postUrlStr = urllib.quote(postUrlStr)
        print(postUrlStr)
        resp = session.post(postUrlStr, data=jsonParam)
        tagValue=resp.text.strip()

        print "tagname:",tag['name'],",tagvalue:", tagValue
        tagList = pydriver.GetTagsByAddr(pkDevice,address)
        pydriver.SetTagsValue(pkDevice,tagList, tagValue)
        pydriver.UpdateTagsData(pkDevice, tagList)

    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    print "-!--OnControl--!-"
    print pkDevice,pkTag
    print "device name:",pkDevice['name'], "tagname:", pkTag['name'], "tagaddress:",pkTag['address'], "tagvalue:",strTagValue
    return 0

