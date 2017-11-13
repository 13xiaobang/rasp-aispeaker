# rasp-aispeaker
use raspberry pi 3 to do as a aispeaker, doing talk and control slave dev (like Android TV).

# 基于raspberry pi 3的家庭智能对话音响

## 1. 概述：
这个project的目的是在raspberry pi 3的硬件上，实现类似天猫精灵/小米AI音响那种，能够智能对话的类音响设备，raspberry pi 3只要在基础版上加上mic扩展板卡(https://www.seeedstudio.com/ReSpeaker-2-Mics-Pi-HAT-p-2874.html)，就可以实现采集语音，再配合一个有线外放，就能完成基本的音频输入输出。配合讯飞/百度等开放的语音接口（ASR/TTS)和开源的webRTC接口（这个project用了webRTC的降噪和VAD模块）,以及类对话机器人系统:图灵机器人（www.tuling123.com）基本能够满足所有语音识别需求。

## 2. 现有功能：
- 唤醒词：“你好音响” （用来控制slave dev）
- 唤醒词：“搜索设备” （用来搜索slave dev）
- 唤醒词：“和我说话” （调用图林机器人实现对话）
- 基本的对话反馈功能

## 3. 关键技术点：
- 语音唤醒：使用了讯飞的离线唤醒, 35天免费试用（http://www.xfyun.cn/index.php/services/awaken）
- 语音降噪：使用了webRTC中的开源代码（在src/libwebrtc里面）
- VAD(语音活动检测:Voice Activity Detection):同样是webRTC中的开源代码（wb_vad.c中)
- ASR(自动语音识别技术Automatic Speech Recognition):用的是百度语音识别（http://ai.baidu.com/sdk#asr）
- 对话机器人：使用的是图灵机器人（www.tuling123.com）
- TTS（从文本到语音(TextToSpeech)）:同样用的是百度（http://ai.baidu.com/tech/speech/tts）
- Android设备端实现接口（代码部分在slave_dev_service）, 该实现在android 设备上验证过。主要是实现一个Android 的native service，在开机时启动，起一个host socket端口监听，当raspberry pi 通过该socket发送有效指令和PCM时就工作，并将PCM传给Android的AudioRecord模块，给设备应用层调用，自己虚拟成一个音频设备。

## 4.交互协议（这里树莓派简化成R, slave Android设备简化成S）
- 初始扫描过程（R和S都开启服务，并在同一网段，但R不知道S的IP，S启动host socekt监听）
    - R扫描整个网段（尝试建立socket，并发送"ask_alive"）
    - 如果在0.5s内，S返回 "is_alive"表明这是个可配对设备。
    - R接着发送ask_bt_addr
    - S返回bt实际的6byte地址
    - 此时R知道S的IP地址以及蓝牙唤醒地址（这里demo设备用的是BLE唤醒）
    - 结束
- 通过R对S进行语音操作（R和S都开启服务，并在同一网段,R知道S的IP，S启动host socekt监听）
    - 用户语音唤醒R后，通过R给S发送了指令。指令以PCM形式被R记录下来。
    - R发送"ask_alive"
    - S返回"is_alive"
    - R发送"start_transfer"并发送raw的PCM数据。S通过keycode触发Android AudioRecord 读取数据，同时S接收数据，通过jni接口供上层同步获取数据
    - R发送完成所有raw PCM后,发送 “stop_transfer”
    - S发送keycode告知AudioRecord完成数据收取工作。
    - S的Android 应用层处理该PCM
    - 结束
- 通过R对S唤醒后，对S进行语音操作（R开启服务，S在待机或关机状态, R知道S的IP和BT地址）
    - 用户语音唤醒R后，通过R给S发送了指令。指令以PCM形式被R记录下来。
    - R发送"ask_alive"
    - S由于待机或关机没有返回"is_alive"
    - R判断S在待机或者关机，发送BLE唤醒信号，唤醒S
    - R不停发送"is_alive"， S启动到Android完成后，发送"is_alive"确认可通信。
    - R发送"start_transfer"并发送raw的PCM数据。S通过keycode触发Android AudioRecord 读取数据，同时S接收数据，通过jni接口供上层同步获取数据
    - R发送完成所有raw PCM后,发送 “stop_transfer”
    - S发送keycode告知AudioRecord完成数据收取工作。
    - S的Android 应用层处理该PCM
    - 结束

## 5 code简易使用过程：
- 程序启动（raspberry部分）：
    - 注册图灵机器人，百度/讯飞等离线接口id，替换ai_speaker.py中的ID号，注册讯飞离线唤醒应用ID，替换pcm_process.c 中“appid = 5a09a164”
    - raspberry 插上语音扩展板以及外放喇叭
    - 将除了slave_dev_service文件夹，其余文件都拷贝到raspberry pi 3任意目录下
    - cd rasp-aispeaker/src/libwebrtc 
    - make
    - cd rasp-aispeaker/src
    - make
    - cd rasp-aispeaker/bin
    - sudo python ai_speaker.py (python2)
- 程序启动（Android slave部分）：
    - 将slave_dev_service拷贝到android project中， source build/envsetup.sh & lunch & mm
    - 将程序编译入target/system中
    - 适配好AudioRecord消息机制部分，主要是keycode和AudioPolicy（这部分因各个平台而异）
    - 启动aispeakerserver（init.rc或手动启动）
- 人机实际交互操作：
    - 说“和我说话” + 任意语音问题， 进行对音响简单对话交互
    - 说“搜索设备”， 会启动设备搜索
    - 说“你好音响” + 任意语音问题，进行对配对slave设备对话交互

