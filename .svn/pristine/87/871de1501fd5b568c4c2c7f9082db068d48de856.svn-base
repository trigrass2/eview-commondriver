#! /usr/bin/python
# coding=utf-8

import sys
import struct
import os
import time

python_file = __file__
bin_path = os.path.dirname(__file__) + '\\..\\..'
python_path = os.path.dirname(__file__) + '\\..\\..\\..\\python'
print(bin_path, python_path)
sys.path.append(bin_path)
sys.path.append(python_path)
print(sys.path)
import pydata
import pydriver
from SqliteHelper import *

def InitDriver(pkDriver):
    print("----InitDriver inspectiondrv----")
    print(pkDriver)
    return 0

def UnInitDriver(pkDriver):
    print("----UnInitDriver inspectiondrv----")
    print(pkDriver)
    return 0

def InitDevice(pkDevice):
    print("----start InitDevice " + pkDevice['name'] + "----")
    db =SqliteHelper()
    print(pkDevice)
    tags = []
    for tag in pkDevice['tags']:
        tags.append(tag)
    print("----end InitDevice" + pkDevice['name'] + "----")
    timerObj=pydriver.CreateTimer(pkDevice, 10000, tags)
    return 0

def UnInitDevice(pkDevice):
    print("UnInitDevice" + pkDevice['name'])
    return 0

def OnTimer(pkDevice, timerObj, pkTimerParam):
    now = float(time.time())
    last = float(pkDevice['param3'])
    print("===lsat inspection time is " + str(time.localtime(last)) + "===")
    cycle = float(pkDevice['param4'])
    print("===inspection cycle is " + str(cycle/60/60/24) + "days===")

    # 开始巡检
    if (now - last >= cycle):
        print("===start inspection===")
        # 读tag初始化点
        handle = pydata.Init("127.0.0.1")
        print("===handle:" + str(handle) + "===")

        # 巡检记录
        record = {}

        # 获取巡检参数
        param = str(pkDevice['param1']) + str(pkDevice['param2'])
        print ("===inspection param is :" + str(param) + "===")
        param_list = param.split(";")

        # 设备状态更新sql数组
        sqlList = [''] * len(param_list)
        sqlList[0] = "update t_system_info set value = 1 where name = 'power_status'"
        sqlList[1] = "update t_matrix_output set signal = 1 where id = 2"
        sqlList[2] = "update t_system_info set value = 0 where name = 'volume_mute'"
        sqlList[3] = "update t_system_info set value = 1 where name = 'volume_mute'"
        sqlList[4] = "update t_system_info set value = '待机' where name = 'dvd_state'"
        sqlList[5] = "update t_system_info set value = '关机' where name = 'dvd_state'"
        sqlList[6] = "update t_system_info set value = 1 where name = 'stagelamp_status'"
        sqlList[7] = "update t_system_info set value = 0 where name = 'stagelamp_status'"
        sqlList[8] = "update t_system_info set value = 1 where name = 'layout_status'"
        sqlList[9] = "update t_system_info set 'value' = 0 where name = 'power_status'"
        # 数据库操作对象
        db = SqliteHelper()

        # 更改巡检状态
        sql = "update t_system_info set 'value' = 1 where name = 'inspection_state'"
        db.execute_update(sql)
        print("===execute sql:" + sql + "===")
        print("update inspection_status success")

        # 发送巡检命令 更新记录字典
        for i in range(0, len(param_list)):
            # 控制点
            tagName = param_list[i].split(":")[0]
            # 控制设备
            devName = tagName.split(".")[0]
            # 控制值
            tagValue = param_list[i].split(":")[1]
            pydata.Control(handle, tagName, tagValue)
            print("===Control tag:"+str(tagName) + "value:" + str(tagValue) + "===")
            time.sleep(20)
            tagValueV = pydata.Get(handle, tagName, "v")[1]
            print("===getDatavalue:"+ str(tagValueV) + "===")
            if (str(tagValueV) == str(tagValue)):
                print("===control tag" + str(i) + "success===")
                # 设备正常
                db.execute_update(sqlList[i])
                print("===execute sql:" + str(sqlList[i]) + "===")
                if (record.has_key(tagName)):
                    record[tagName] += 0
                else:
                    record[tagName] = 0
            else:
                print("===control tag" + str(i) + "fail===")
                db.execute_update(sqlList[i])
                print("===execute sql:" + str(sqlList[i]) + "===")
                # 设备异常
                if (record.has_key(tagName)):
                    record[tagName] += 1
                else:
                    record[tagName] = 1
        record["time"] = time.strftime("%Y%m%d %H:%M:%S")

        # insert巡检记录
        sql = "insert into t_inspect_history ("
        devList = list(record.keys())
        sql += (devList[0].split('.'))[0]
        for i in range(1, len(devList)):
            sql += "," + (devList[i].split('.'))[0]
        sql += ") values ('" + str(record[devList[0]])
        for i in range(1, len(devList)):
            sql += "','" + str(record[devList[i]])
        sql += "')"
        print("===execute sql:" + str(sql) + "===")
        db.execute_update(sql)

        # update巡检时间
        sql = "update t_device_list set 'param3' = " + str(now) + " where name = '" + pkDevice['name'] + "'"
        db.execute_update(sql)
        print("===execute sql:" + sql + "===")
        print ("===inspection success" + str(time.localtime(now)) + "===")

        # 更改巡检状态
        sql = "update t_system_info set 'value' = 0 where name = 'inspection_state'"
        db.execute_update(sql)
        print("===execute sql:" + sql + "===")
        print("update inspection_status success")

        pydata.Exit(handle)
    return 0

def OnControl(pkDevice, pkTag, strTagValue):
    now = float(time.time())
    last = float(pkDevice['param3'])
    print("===lsat inspection time is " + str(time.localtime(last)) + "===")

    # 开始巡检
    print("===start inspection===")
    # 读tag初始化点
    handle = pydata.Init("127.0.0.1")
    print("===handle:" + str(handle) + "===")

    # 巡检记录
    record = {}

    # 获取巡检参数
    param = str(pkDevice['param1']) + str(pkDevice['param2'])
    print ("===inspection param is :" + str(param) + "===")
    param_list = param.split(";")

    # 设备状态更新sql数组
    sqlList = [''] * len(param_list)
    sqlList[0] = "update t_system_info set value = 1 where name = 'power_status'"
    sqlList[1] = "update t_matrix_output set signal = 1 where id = 2"
    sqlList[2] = "update t_system_info set value = 0 where name = 'volume_mute'"
    sqlList[3] = "update t_system_info set value = 1 where name = 'volume_mute'"
    sqlList[4] = "update t_system_info set value = '待机' where name = 'dvd_state'"
    sqlList[5] = "update t_system_info set value = '关机' where name = 'dvd_state'"
    sqlList[6] = "update t_system_info set value = 1 where name = 'stagelamp_status'"
    sqlList[7] = "update t_system_info set value = 0 where name = 'stagelamp_status'"
    sqlList[8] = "update t_system_info set value = 1 where name = 'layout_status'"
    sqlList[9] = "update t_system_info set 'value' = 0 where name = 'power_status'"
    # 数据库操作对象
    db = SqliteHelper()

    # 更改巡检状态
    sql = "update t_system_info set 'value' = 1 where name = 'inspection_state'"
    db.execute_update(sql)
    print("===execute sql:" + sql + "===")
    print("update inspection_status success")

    # 发送巡检命令 更新记录字典
    for i in range(0, len(param_list)):
        # 控制点
        tagName = param_list[i].split(":")[0]
        # 控制设备
        devName = tagName.split(".")[0]
        # 控制值
        tagValue = param_list[i].split(":")[1]
        pydata.Control(handle, tagName, tagValue)
        print("===Control tag:"+str(tagName) + "value:" + str(tagValue) + "===")
        time.sleep(20)
        tagValueV = pydata.Get(handle, tagName, "v")[1]
        print("===getDatavalue:"+ str(tagValueV) + "===")
        if (str(tagValueV) == str(tagValue)):
            print("===control tag" + str(i) + "success===")
            # 设备正常
            db.execute_update(sqlList[i])
            print("===execute sql:" + str(sqlList[i]) + "===")
            if (record.has_key(tagName)):
                record[tagName] += 0
            else:
                record[tagName] = 0
        else:
            print("===control tag" + str(i) + "fail===")
            db.execute_update(sqlList[i])
            print("===execute sql:" + str(sqlList[i]) + "===")
            # 设备异常
            if (record.has_key(tagName)):
                record[tagName] += 1
            else:
                record[tagName] = 1
    record["time"] = time.strftime("%Y%m%d %H:%M:%S")

    # insert巡检记录
    sql = "insert into t_inspect_history ("
    devList = list(record.keys())
    sql += (devList[0].split('.'))[0]
    for i in range(1, len(devList)):
        sql += "," + (devList[i].split('.'))[0]
    sql += ") values ('" + str(record[devList[0]])
    for i in range(1, len(devList)):
        sql += "','" + str(record[devList[i]])
    sql += "')"
    print("===execute sql:" + str(sql) + "===")
    db.execute_update(sql)

    # update巡检时间
    sql = "update t_device_list set 'param3' = " + str(now) + " where name = '" + pkDevice['name'] + "'"
    db.execute_update(sql)
    print("===execute sql:" + sql + "===")
    print ("===inspection success" + str(time.localtime(now)) + "===")

    # 更改巡检状态
    sql = "update t_system_info set 'value' = 0 where name = 'inspection_state'"
    db.execute_update(sql)
    print("===execute sql:" + sql + "===")
    print("update inspection_status success")

    pydata.Exit(handle)
    return 0