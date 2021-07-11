# coding=utf-8
import sys
import os
import requests
curfilepath=os.path.abspath('.')
sys.path.append(curfilepath + '\\..\\..')
sys.path.append(curfilepath + '\\..\\..\\python')

import sys
import io
import json
#loginurl:http://192.168.0.132:24080/login/manage/login?next=/login/manage
#data:{username:rest,password:nav,login:login}或者undefined或空字符串
def login_post(pkDevice, loginurlStr,loginParamStr,hopeResultStr):
    #sys.stdout = io.TextIOWrapper(sys.stdout.buffer,encoding='utf-8')
    print("login_post:", loginParamStr)
    jsonData={}
    if(loginParamStr is None or loginParamStr==''):
        jsonData={}
    else:
        jsonData = json.loads(loginParamStr)
    headers = {'User-agent':'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.113 Safari/537.36'}
    #login_url = 'http://10.6.0.106/login/manage/login?next=/login/manage'

    login_url = loginurlStr

    session = pkDevice["session"]
    resp = session.post(login_url,data=jsonData)
    print "post url:", login_url, loginParamStr, "result", resp.text

    #resp.text
    if(hopeResultStr is not None and hopeResultStr.strip() != ''):
        pos = resp.text.find(hopeResultStr)
        if(pos >= 0):
            return True
        else:
            return False

    return True
#
#print(resp.content.decode('utf-8'))

