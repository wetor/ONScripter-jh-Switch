

# **ONSCripter-jh for Nintendo Switch**
## **简介**
- ONScripter（Open Source Nscripter ）是一个用来解析NScripter脚本的第三方开源Galgame游戏引擎，可以在多种平台和设备上（Windows，Linux，FreeBSD，Android，iOS，Symbian……etc）上运行使用NScripter脚本编译的文字冒险类游戏。
- ONScripter-Jh是jh10001在原有的ONS基础上进行修改，修改目标： 提供比原版ONScripter更好的性能，适当增加一些功能 添加中文支持 尽可能的兼容原版ONS脚本。
- ONScripter-Jh for PSVita是wetor即本人移植的jh版的SDL2分支，用于在PSVita平台运行ONS游戏。
- ONScripter-Jh for Switch是wetor即本人移植的jh版的SDL2分支，用于在Nintendo Switch平台运行ONS游戏。

## 博客链接
https://blog.wetorx.cn/post/onscripter-ns.html

## 更新日志 Update logs

### 2021.7.5
- Support English games
- Usage : 
  1. Put the "onsemu" in the "SDFile" folder into the root directory of the sd card
  2. Install "ONSBrowser-install-05293394190000.nsp"
  3. Put the ONS games folder in "onsemu"
  4. Enjoy !

### 2021.7.3  GUI ver.2.0
- **修复12.0以上系统无法打开的问题**
- 修复退出游戏无法返回GUI并且报错的问题


### 2019.10.3 GUI ver.1.1
- 修复游戏数量小于5个时打开启动器错误的问题
- 修复部分游戏存档错误问题

## 关于启动器GUI
如截图1 2所示  
这个启动器的界面是我从零开始写的，借鉴了很多开源的ns自制程序，当时c++编码能力，代码逻辑比较乱  
https://github.com/wetor/ONS-Switch-GUI  

## **关于Switch版**

  模拟器本体几个月前就完成了...做了个没什么用的浏览器，因为NS已经可以刷安卓了，这种模拟器当然不在话下，所以也没有继续完善的必要了。

  NS比起PSV那孱弱的性能来说真的好太多了，稍微重写了一些地方，ONS就能运行了，也不会出现PSV那种因为IO速度限制导致的卡顿

与其继续研究这个，不如等国外的那个大佬把kirikiroid2移植完再说吧（krkr已经移植到NS了，github仓库名krkrs）。

至于那个ONSBrowser，是用Plutonium自己写的，用了很长时间...纯属娱乐

[**下载地址**](#下载地址)

[**安装说明**](#安装说明)

**ONS for Switch演示视频：https://www.bilibili.com/video/av68622183/**

**ONS for Switch项目：https://github.com/wetor/ONScripter-jh-Switch**


## **截图**

<img src="./screenshot/2.jpg" width="50%" height="50%" />
<img src="./screenshot/3.jpg" width="50%" height="50%" />
<img src="./screenshot/4.jpg" width="50%" height="50%" />
<img src="./screenshot/5.jpg" width="50%" height="50%" />
<img src="./screenshot/1.jpg" width="50%" height="50%" />

## 下载地址
**请先查看安装说明**

**github下载: https://github.com/wetor/ONScripter-jh-Switch/releases/latest**

或者

**链接: https://pan.baidu.com/s/1lFaTjYLOPluEe8YFBPmW4w 提取码:e8q7**


## 安装说明
### **安装模拟器**

1. 将SDFile文件夹中的onsemu放入内存卡根目录，即【和switch文件夹同级】
2. 安装ONSBrowser-install-0529293394190000.nsp，桌面即可出现快捷方式。【安装时显示1.0版本，如已安装旧版本，可强制安装，或者先删除旧版再安装】

### **安装游戏**

- 从网上下载ONS游戏包，不区分平台，只要是ONScripter模拟器能运行的游戏资源就可以。
- 检查游戏资源是否完整，简单的辨别方法：一定存在'0.txt'、'00.txt'、'nscript.dat'其中之一，可能存在'\*.nsa'、'\*.sar'文件，大部分存在'icon0.png'。
- 将游戏资源文件夹（不允许存在二级目录）用英文字符重命名，不能用中文字符，放至SD卡的'onsemu'文件夹中，如'SDCard:/onsemu/Rewrite/00.txt'，文件夹名称将作为选择游戏时的重要标志，安装游戏完成。
- 注1：游戏文件夹不能存在中文字符！不能中文！不能中文！否则将无法正常识别。
- 注2：如以上均无误，但是启动游戏后出错，可以使用其他平台的ONS模拟器运行同一资源包实验，如其他平台均无问题，可向我反馈，需提供游戏资源包。


## 使用说明

- ONSBrowser中对快捷键均有说明，可在选择游戏前按下L键查看帮助。
- 目前Y键的查看详细信息、X键的浏览游戏资源、R键的视频播放器功能均未实现，有生之年在加上，去掉了不好看，按钮就留着了= =。
- 关于游戏名称显示问题，ONSBrowser显示的均是文件夹名称，实际名称由于编码问题无法正常显示。
- 部分游戏由于脚本问题，按键效果可能不会太符合，但大多数都是没错的。

## 快速启动
### **制作游戏的单独前端**

[使用Releases中附带的工具](https://github.com/wetor/ONScripter-jh-Switch/releases/latest)

工具来源：https://gitlab.com/martinpham/NSP-Forwarder

### **修改源代码**
修改模拟器源代码，将游戏资源打包进romfs，即可实现独立游戏的打包。

**libnx版本：4.0.1-1**

## **相关链接**
原版ONScripter官网：https://onscripter.osdn.jp/onscripter.html

ONScripter-Jh项目：https://bitbucket.org/jh10001/onscripter-jh

ONS for PSVita：http://www.wetor.top/onscripter-psvita.html

ONS for PSVita项目：https://github.com/wetor/ONScripter-jh-PSVita

**ONS for Switch演示视频：https://www.bilibili.com/video/av68622183/**

**ONS for Switch项目：https://github.com/wetor/ONScripter-jh-Switch**

**ONS for Switch发布页：http://www.wetor.top/onscripter-ns.html**

