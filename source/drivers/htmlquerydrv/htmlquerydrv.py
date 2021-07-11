#! /usr/bin/python
# coding=utf-8

import sys
import os
import struct
import urllib2
import requests

sys.path.append(os.path.abspath('.') + '\\..\\..')
sys.path.append(os.path.abspath('.') + '\\..\\..\\python')
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
    pageName2Tags={}
    for tag in pkDevice['tags']:
        address = tag["address"]
        addrArr = address.split(';', 1) #PAGEURL;jQueryStr
        pageUrl=addrArr[0]
        jQueryStr=""
        if(len(addrArr) > 1):
            jQueryStr = addrArr[1]
        tags = pageName2Tags.get(pageUrl, None)
        if(tags==None):
            tags=[]
            tags.append(tag)
            pageName2Tags[pageUrl]  = tags
        else:
            tags.append(tag)

    #print "=========device tags end ========"
    for pageName,tags in pageName2Tags.items():
        print "pageUrl:", pageName, "tagcount:", len(tags)
        timerObj=pydriver.CreateTimer(pkDevice, 3000, (pageName,tags))

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
    loginUrlStr = pkDevice["connparam"] + "/" + loginUrl
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

#pkTimerParam={pageName,tags}
def OnTimer(pkDevice, timerObj, pkTimerParam):
    print "----OnTimer----"
    pageName=pkTimerParam[0]
    tags=pkTimerParam[1]
    session = pkDevice["session"]
    connparam=pkDevice["connparam"]
    pageUrlStr = connparam+"/"+pageName
    #req_sgu_url = 'http://' + "127.0.0.1" + '/VRB-53D%20DVOR-SGU.html'
    resp = session.get(pageUrlStr)
    pqQuery = pq(resp._content)
    for tag in tags:
        address = tag['address']
        pageNameInAddr = address.split(';')[0]
        jqueryStr = address.split(';')[1]
        pqQueryStr = "pqQuery" + jqueryStr
        tagValue = eval(pqQueryStr)
        #去除空格
        tagValue=tagValue.strip()

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

