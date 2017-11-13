# Copyright (C) 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Helper functions for audio streams."""
# -*- coding:utf-8 -*- 
import logging
import threading
import time
import wave
import math
import array
import socket
import sys
import thread
import click
import sounddevice as sd
from aip import AipSpeech
import json
import os
import urllib,urllib2
import fcntl
import struct
from IPy import IP

from pydub import AudioSegment
from audio_stream import *


DEFAULT_AUDIO_SAMPLE_RATE = 16000
DEFAULT_AUDIO_SAMPLE_WIDTH = 2
DEFAULT_AUDIO_ITER_SIZE = 3200
DEFAULT_AUDIO_DEVICE_BLOCK_SIZE = 6400
DEFAULT_AUDIO_DEVICE_FLUSH_SIZE = 25600

reload(sys)  
sys.setdefaultencoding('utf8')

# need register 1: baidu ASR 
BAIDU_APP_ID = '10120974'
BAIDU_API_KEY = 'RqYNdOd0dLQaYLy1p9I3zy62'
BAIDU_SECRET_KEY = '2ba4ad4e5bc3a6d84f7df0d6bcf11f37'
BAIDU_aipSpeech = AipSpeech(BAIDU_APP_ID, BAIDU_API_KEY, BAIDU_SECRET_KEY)

#need register 2: TULIN talk  http://www.tuling123.com/
TULIN_API_KEY = '4779ef9a644244e8865d911f5952dc6a'
TULIN_URL = "http://47.94.75.128/openapi/api?key=%s&info=" % TULIN_API_KEY

#need register 3: baidu tts
BAIDU_TTS_APP_ID = '10205809'
BAIDU_TTS_API_KEY = 'W8HqNvw0BosgXd0eoxQeQxeq'
BAIDU_TTS_SECRET_KEY = 'CtEyonALjCrmgbLHAQQL7dKNEtrqH4WQ'
BAIDU_TTS_aipSpeech = AipSpeech(BAIDU_TTS_APP_ID, BAIDU_TTS_API_KEY, BAIDU_TTS_SECRET_KEY)

#global param#############################################################

g_status = 7 # 0: acvite and idle.
             # 3: close. 
             # 4: slave_dev can not ping alive
             # 5: slave_dev eth is alive, but slave_dev system is not alive.
             # 6: slave_dev is pending: when wakeup voice det, slave_dev is not active and idle, it will be set here.
             # 7: init
             # 8: talking mode begin.
             # 9: talking mode vad det.

g_pcm_status = 0 # 0: idle
                    # 1: wake up detect
                    # 2: vad stop detect. will back to 0
                    # 3: close.
                    # 6: xunfei wakeup & VAD is done,  and blocking because slave_dev last time is not transfered.
                    # 8: talking mode begin.
                    # 9: talking mode vad det.

g_wait_slave_dev_up = 0 # 0 : not waiting
                  # 1 : is waiting

g_slave_dev_eth_link_up = 0   # 0: link not esiblashed, can ping , and tcpCliSock has connected
                        # 1: link esiblashed

g_wav_mutex = threading.Lock()  
tcpCliSock = 0
stream = 0

TV_IP = ""
TV_PORT = ""
TV_BT_ADDR = ""
TV_BT_TYPE =""
TV_SOCK_ADDR = ""

wav_files = (
            './wav_file/hello.pcm', #0
            './wav_file/knowit.pcm',#1
            './wav_file/wakeup.pcm', #2
            './wav_file/abnormal_wakeup.pcm',#3
            './wav_file/wakeup_vt.pcm',#4
            './wav_file/listening.pcm',#5
            './wav_file/dontknow.pcm',#6
            './wav_file/searching.pcm',#7
            './wav_file/search_done.pcm',#8
            './wav_file/search_ip.pcm', #9
            './wav_file/search_not_found.pcm', #10
            )

config_file = "./param.cfg"

#global param#############################################################

# get file content from "filePath"
def get_file_content(filePath):
    with open(filePath, 'rb') as fp:
        return fp.read()

# play "wav_num" of wav_files
def play_wav(wav_num):
    global g_wav_mutex,wav_files,stream
    if g_wav_mutex.acquire():
        print("do play wav***********************")
        stream.stop_recording()
        stream.start_playback()
        stream.write(get_file_content(wav_files[wav_num]))
        stream.stop_playback()
        stream.start_recording()
    g_wav_mutex.release()
    return

#use BLE tool "hcitool" to wake up slave_dev:
def wake_up_slave_dev():
    print("TV_BT_ADDR=%s"%(TV_BT_ADDR))
    os.system('sudo hcitool cmd 0x08 0x000A 0x00')
    os.system('sudo hcitool -i hci0 cmd 0x08 0x0006 20 00 20 00 00 00 00 00 00 00 00 00 00 07 03')
    str = 0;
    if(TV_BT_TYPE == "bcm"):
        str = 'sudo hcitool -i hci0 cmd 0x08 0x0008 17 02 01 02 13 FF 0F 00 42 52 43 '
        str += TV_BT_ADDR
        str += " 0C 0E 00 3B A9 FD C5"
    else:
        str = 'sudo hcitool -i hci0 cmd 0x08 0x0008 14 13 FF 0F 00 42 52 43 4D '
        str += TV_BT_ADDR
        str += ' 04 00 FF FF FF FF'
    os.system(str)
    os.system('sudo hcitool cmd 0x08 0x000A 0x01')
    time.sleep(3)
    os.system('sudo hcitool cmd 0x08 0x000A 0x00')

#get ipaddr from "ifname"
def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
    )[20:24])

#get netmask from "ifname"
def get_netmask_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x891b, #SIOCGIFNETMASK
        struct.pack('256s', ifname[:15])
    )[20:24])

#write ip/bt_addr/bt_type to config file.
def write_param_file(ip, bt_addr, bt_type):
    f=open(config_file,'w')
    f.write("ip="+ip+'\n')
    f.write("port=5678\n")
    f.write("bt_addr="+bt_addr+'\n')
    f.write("bt_type="+bt_type+'\n')
    f.close()

#use baidu tts to generate voice of "ip", save it in ./wav_file/search_ip.pcm
def generate_ip_wav(ip):
    result  = BAIDU_TTS_aipSpeech.synthesis(ip, 'zh', 1, {'vol': 5,})
    with open('temp.mp3', 'wb') as f:
        f.write(result)
    sound = AudioSegment.from_mp3("temp.mp3")
    mono = sound.set_frame_rate(16000).set_channels(1)
    mono.export("./wav_file/search_ip.pcm", format="s16le")
    print("pcm generate done")

#search devices in local ip address field.
#   for example:  raspberry ai speaker ip is "192.168.1.20",
#                  scan from "192.168.1.0" ~ "192.168.1.255"
#                 to search for slave_dev.
def search_devices():
    global g_wav_mutex
    global TV_IP,TV_BT_ADDR,TV_PORT,TV_BT_TYPE,TV_SOCK_ADDR
    play_wav(7)
    ret = 1
    g_wav_mutex.acquire()
    ip = get_ip_address('eth0')
    netmask = get_netmask_address('eth0')
    whole_ips = (IP(ip).make_net(netmask))

    print("begin to search device!!!")
    for x in whole_ips:
        try:
            ADDR=(str(x), 5678)
            localSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            localSock.settimeout(0.5)
            localSock.connect(ADDR)
            localSock.send("ask_alive")
            recv_temp = localSock.recv(20)
            if (recv_temp == "is_alive"):
                print("%s is active"%(x))
                localSock.settimeout(2)
                localSock.connect(ADDR)
                localSock.send("ask_bt_addr")
                recv_temp = localSock.recv(32)
                print("recv_temp = %s"%(recv_temp))
                if(recv_temp == "no_bt_addr"):
                    print("no_bt_addr!")
                    sys.exit(0)
                elif(recv_temp.find("bcm")>=0):
                    print("bcm: %s"%(recv_temp))
                elif(recv_temp.find("rtk")>=0):
                    print("rtk: %s"%(recv_temp)) 
                write_param_file(str(x), recv_temp[3:], recv_temp[0:3])
                TV_PORT = 5678
                TV_IP=str(x)
                TV_BT_TYPE = recv_temp[3:]
                TV_BT_ADDR = recv_temp[0:3]
                TV_SOCK_ADDR = (TV_IP,TV_PORT)
                generate_ip_wav(TV_IP)
                read_param_cfg(config_file)
                ret=0
                break
            else:
                print("%s is not active"%(x))
        except Exception as e:
            print("except: %s not connectable!"%(x))
            print(e)
            localSock.close()
    g_wav_mutex.release()
    if(ret==0):
        play_wav(8)
        play_wav(9)
    else:
        play_wav(10)
    return ret

# reverse the bt addr:
# 11:22:33:44:55:66 -> 66:55:44:33:22:11
def reverse_bt_addr(addr):
    temp = ""
    for i in range(0,6):
        temp += addr[15-i*3];
        temp += addr[15-i*3+1];
        temp += ' ';
    temp = temp[0:len(temp)-1]
    print(temp)
    return temp


#get param contest:
#for example:
#ip=192.168.1.1
#port=5678
#bt_addr=48:a9:d2:7d:1c:c6
#bt_type=bcm
#
def read_param_cfg(cfg_path):
    global TV_IP,TV_BT_ADDR,TV_PORT,TV_BT_TYPE,TV_SOCK_ADDR
    try:
        file_obj = open(cfg_path)
        while(1):
            line = file_obj.readline();
            if not line:
                break
            line = line.strip('\n')
            if(line.startswith("ip=")):
                TV_IP=line[3:]
            elif(line.startswith("bt_addr=")):
                TV_BT_ADDR=line[8:]
                TV_BT_ADDR=TV_BT_ADDR.replace(":", " ")
            elif(line.startswith("port=")):
                TV_PORT=line[5:]
                TV_PORT=int(TV_PORT)
            elif(line.startswith("bt_type=")):
                TV_BT_TYPE=line[8:]
            else:
                print("py: undefined param in param.cfg")
                file_obj.close()
                return 1
        #print("TV_IP={0}, TV_PORT={1}".format(str(TV_IP),str(TV_PORT)))
        if(TV_BT_TYPE=="bcm"):
            TV_BT_ADDR = reverse_bt_addr(TV_BT_ADDR)
        print("TV_IP=%s"%(TV_IP))
        print("TV_PORT=%s"%(TV_PORT))
        print("TV_BT_ADDR=%s"%(TV_BT_ADDR))
        print("TV_BT_TYPE=%s"%(TV_BT_TYPE))
        TV_SOCK_ADDR = (TV_IP, TV_PORT)
        file_obj.close()
    except Exception as e:
        print("py: param.cfg read error")
        print(e)
        return 1
    return 0

# after slave_dev wakeup and boot successfully, push
# the saved pcm to slave_dev
def triggle_push_saved_pcm(tcpCliSock1, ADDR):
    global g_status
    global g_pcm_status
    global tcpCliSock
    global TV_IP, TV_SOCK_ADDR
    while(g_pcm_status != 6):
        time.sleep(1)
    print("start push last time pcm,wait 3s")
    time.sleep(3)
    tcpCliSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcpCliSock.connect(TV_SOCK_ADDR)
    tcpCliSock.send("wake_up")
    tcpCliSock.send("start_transfer")
    time.sleep(1)
    tcpCliSock.send(get_file_content('out.pcm'))
    time.sleep(1)
    tcpCliSock.send("stop_transfer")
    tcpCliSock.close()
    print("end push last time pcm!")
    g_pcm_status = 0
    g_status = 0

# wait slave_dev wake up
# 1. ping the slave_dev ip
# 2. send "ask_alive", expect receive "is_alive" if wake up successfully.
def wait_slave_dev_up(tcpCliSock1, HOST,ADDR):
    global g_status
    global g_wait_slave_dev_up
    global g_slave_dev_eth_link_up
    global tcpCliSock
    global TV_IP, TV_SOCK_ADDR
    if(g_wait_slave_dev_up == 1):
        return
    g_wait_slave_dev_up = 1
    while (os.system('ping ' + TV_IP + " -c 1 -W 1") != 0):
        print("slave_dev eth is not alive !, wait...")
    if (g_slave_dev_eth_link_up == 0):
        g_slave_dev_eth_link_up = 1
        while True:
            try:
                tcpCliSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                tcpCliSock.settimeout(1)
                tcpCliSock.connect(TV_SOCK_ADDR)
                tcpCliSock.send("ask_alive")
                recv_temp = tcpCliSock.recv(20)
                if "is_alive" == recv_temp:
                    print("slave_dev is OK!!!!!!!!!!!!")
                    break
                elif "not_alive" == recv_temp :
                    time.sleep(1)
                else:
                    print("slave_dev system is not active !, wait...")
                    print("recv=%s"%(recv_temp))
                tcpCliSock.close()
            except IOError:
                print("maybe broken pipe, retry connect")
                tcpCliSock.close()
            except:
                print("other exceptions")
                
    print("slave_dev is active and idle now!")
    if(g_status == 6):
        triggle_push_saved_pcm(tcpCliSock, TV_SOCK_ADDR)
    g_wait_slave_dev_up = 0     


# start a thread, process status machine with "pcm_process"
def listen_sig(name, sock):
    global g_status
    global g_pcm_status
    global g_wait_slave_dev_up
    global g_slave_dev_eth_link_up
    global tcpCliSock
    global TV_IP, TV_SOCK_ADDR
    print ("fuc_name=%s"%(name))
    while True:
        try:
            reply=sock.recv(20)
            print ("reply=%s"%(reply))
            if (g_pcm_status == 0 and "wake_up0" == reply):
                print("wakeup0 det")
                if(g_status == 6):
                    print("last time pcm is blocking, ignore it")
                    continue
                else:
                    play_wav(0)
                    tmp_status = check_slave_dev_alive()
                    if(tmp_status == 0):
                        g_status = 0
                        tcpCliSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        tcpCliSock.connect(TV_SOCK_ADDR)
                        tcpCliSock.send("wake_up")
                        tcpCliSock.send("start_transfer")
                    else:
                        g_status = 6
                        thread.start_new_thread(wake_up_slave_dev, ())
                        thread.start_new_thread(wait_slave_dev_up, (tcpCliSock, TV_IP,TV_SOCK_ADDR))
                    g_pcm_status=1
            elif(g_pcm_status == 0 and "wake_up1" == reply):
                print("wakeup1 det")
                #thread.start_new_thread(search_devices, ())
                search_devices()                    
                #play_wav(7)
            elif(g_pcm_status == 0 and "wake_up2" == reply):
                print("wakeup2 det");
                play_wav(5)
                g_status = 8
                g_pcm_status = 8
            elif(g_pcm_status == 8  and "vad_det" == reply):
                g_status = g_pcm_status = 9
            elif(g_pcm_status == 0 and "abnormal_wakeup" == reply):
                if(g_status == 6):
                    print("last time pcm is blocking, ignore it")
                    continue
                else:
                    play_wav(3)
            elif(g_pcm_status == 1 and "vad_det" == reply):
                g_pcm_status = 2
                play_wav(1)
            else:
                print("g_pcm_status = %d, socket maybe close1"%(g_pcm_status))
                g_pcm_status = 3
                sys.exit(0)
                break
                #print("false status!!!!")
        except Exception as e:
            print("socket maybe close2")
            g_pcm_status = 3
            sys.exit(0)
            break

#check whether slave_dev is alive, return g_status.
def check_slave_dev_alive():
    temp_g_status = 0
    global g_wait_slave_dev_up
    global g_slave_dev_eth_link_up
    global g_status
    global tcpCliSock
    global TV_IP, TV_SOCK_ADDR
    exit_code = os.system('ping ' + TV_IP + " -c 1 -W 1")
    if exit_code:
        print("slave_dev is not alive!")
        g_slave_dev_eth_link_up = 0
        temp_g_status = 4
    else:
        if((g_status != 0 or g_status != 5) and g_slave_dev_eth_link_up == 0):
            tcpCliSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcpCliSock.settimeout(1)
            tcpCliSock.connect(TV_SOCK_ADDR)
            g_slave_dev_eth_link_up = 1
            tcpCliSock.send("ask_alive")
            recv_temp = tcpCliSock.recv(20)
            tcpCliSock.close()
            if (recv_temp == "is_alive"):
                print("slave_dev is active")
                temp_g_status = 0
            else:
                temp_g_status = 5
        elif (g_status == 5):
            tcpCliSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcpCliSock.settimeout(1)
            tcpCliSock.connect(TV_SOCK_ADDR)
            recv_temp = tcpCliSock.recv(20)
            tcpCliSock.close()
            if (recv_temp == "is_alive"):
                print("slave_dev is active")
                temp_g_status = 0
            else:
                temp_g_status = 5
    print("slave_dev status is %d"%(temp_g_status))
    return temp_g_status

#start pcm process(c code)
def start_pcm_thread():
    print("start pcm thread!")
    os.putenv('LD_LIBRARY_PATH',"../libs/x86:../src/libwebrtc/lib")
    os.system('./pcm_process')

#do TULIN TTS process
#  baidu asr result -> tulin talk -> baidu tts(mp3) -> pcm result
def TULIN_TTS_process(i):
    TULINURL = "%s%s" % (TULIN_URL,urllib2.quote(i.encode('utf-8')))
    req = urllib2.Request(url=TULINURL)
    result = urllib2.urlopen(req).read()
    hjson=json.loads(result)
    length=len(hjson.keys())
    content=hjson['text']
    if length==3:
        i=content+hjson['url']
    elif length==2:
        i=content
    print("tulin talk done.");
   
    result  = BAIDU_TTS_aipSpeech.synthesis(i, 'zh', 1, {'vol': 5,})
    with open('temp.mp3', 'wb') as f:
        f.write(result)
    sound = AudioSegment.from_mp3("temp.mp3")
    mono = sound.set_frame_rate(16000).set_channels(1)
    mono.export("temp.pcm", format="s16le")
    print("pcm generate done")
    with open("temp.pcm","rb") as f:
        result = f.read()
    global g_wav_mutex,wav_files,stream
    if g_wav_mutex.acquire():
        print("do play wav***********************")
        stream.stop_recording()
        stream.start_playback()
        stream.write(result)
        stream.stop_playback()
        stream.start_recording()
    g_wav_mutex.release()

@click.command()
@click.option('--record-time', default=10,
              metavar='<record time>', show_default=True,
              help='Record time in secs')
@click.option('--audio-sample-rate',
              default=DEFAULT_AUDIO_SAMPLE_RATE,
              metavar='<audio sample rate>', show_default=True,
              help='Audio sample rate in hertz.')
@click.option('--audio-sample-width',
              default=DEFAULT_AUDIO_SAMPLE_WIDTH,
              metavar='<audio sample width>', show_default=True,
              help='Audio sample width in bytes.')
@click.option('--audio-iter-size',
              default=DEFAULT_AUDIO_ITER_SIZE,
              metavar='<audio iter size>', show_default=True,
              help='Size of each read during audio stream iteration in bytes.')
@click.option('--audio-block-size',
              default=DEFAULT_AUDIO_DEVICE_BLOCK_SIZE,
              metavar='<audio block size>', show_default=True,
              help=('Block size in bytes for each audio device '
                    'read and write operation..'))
@click.option('--audio-flush-size',
              default=DEFAULT_AUDIO_DEVICE_FLUSH_SIZE,
              metavar='<audio flush size>', show_default=True,
              help=('Size of silence data in bytes written '
                    'during flush operation'))
def main(record_time, audio_sample_rate, audio_sample_width,
         audio_iter_size, audio_block_size, audio_flush_size):
    """Helper command to test audio stream processing.

    - Record 5 seconds of 16-bit samples at 16khz.
    - Playback the recorded samples.
    """
    global g_status
    global g_pcm_status
    global g_wait_slave_dev_up
    global g_slave_dev_eth_link_up
    global tcpCliSock
    global g_wav_mutex
    global stream

    thread.start_new_thread(start_pcm_thread,())
    time.sleep(1)
    
    audio_device = SoundDeviceStream(sample_rate=audio_sample_rate,
                                     sample_width=audio_sample_width,
                                     block_size=audio_block_size,
                                     flush_size=audio_flush_size)
    stream = ConversationStream(source=audio_device,
                                sink=audio_device,
                                iter_size=audio_iter_size,
                                sample_width=audio_sample_width)
    
    ################slave_dev client socket
    global TV_IP,TV_BT_ADDR,TV_PORT,TV_SOCK_ADDR
    if(read_param_cfg(config_file)==1):
        TV_PORT = 5678
        TV_IP="172.16.126.7"
        TV_BT_TYPE = "rtk"
        TV_BT_ADDR = "1C 1E E3 4B E1 05"
        TV_SOCK_ADDR = (TV_IP,TV_PORT)    
    check_slave_dev_alive()
    ################

    #################socket
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    server_address = "./CAN_SERVICE"
    print >>sys.stderr, 'connecting to %s' % server_address  
    try:
        sock.connect(server_address)
    except socket.error, msg:  
        print >>sys.stderr, msg 
        sys.exit(1)
    
    thread.start_new_thread(listen_sig, ("listen_sig_name", sock))  
    ################

    samples =[]
    logging.basicConfig(level=logging.INFO)
    logging.info('Starting audio test.')

    stream.start_recording()
    logging.info('Recording samples.')
    
    #while time.time() < end_time:
    while True:
        try:
            if(g_wav_mutex.acquire()):
                pcm_buffer = stream.read(audio_block_size)
            g_wav_mutex.release()
            if(g_pcm_status == 6):
                continue #block all the video if pcm blocking.

            if(g_pcm_status == 0):
                print("say something to wakeup...");
            else:
                print("say voice cmd...");
                
            sock.send(pcm_buffer)
            if (g_pcm_status == 8):#wake detect
                samples.append(pcm_buffer)
            if (g_pcm_status == 1):
                if(g_status == 0):
                    tcpCliSock.send(pcm_buffer)
                samples.append(pcm_buffer)
            if (g_pcm_status == 2 or g_pcm_status == 9):#vad detect
                if g_status == 6:
                    play_wav(2)
                #1. save the file
                if( len(samples)>40): #do not bigger than 256k
                    samples = samples[len(samples)-40:len(samples)]
                f = open("out.pcm", "wb")
                while len(samples):
                    temp_buf = samples.pop(0)
                    f.write(temp_buf)
                f.close()

                if(g_status != 6 and g_status != 9):
                    tcpCliSock.send("stop_transfer")
                    tcpCliSock.close()
                
                #3. init buffer
                del samples[:]
                if(g_status == 6):
                    g_pcm_status = 6
                else:
                    g_pcm_status = 0
                    
                #2. do baidu ASR
                result=BAIDU_aipSpeech.asr(get_file_content('out.pcm'), 'pcm', 16000, {'lan': 'zh',})
                json_result = json.dumps(result)
                strtestObj = json.loads(json_result)
                lists = strtestObj["result"]
                for i in lists:
                    print (i)
                    if(g_status == 9):
                        if(i):
                            TULIN_TTS_process(i)
                        else:
                            play_wav(6)
                        g_pcm_status = g_status = 0
                
            if (g_pcm_status == 3):
                sys.exit(0)
        except Exception as e:
            if(g_status == 9):
                g_pcm_status = g_status = 0
                print("NLP error:")
                print(e)
                play_wav(6)
            else:
                print("get exception, may be baidu NLP fails, ignore it.")
                print(e)

    sock.close

if __name__ == '__main__':
    main()
