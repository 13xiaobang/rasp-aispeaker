#/usr/bin/env python
# -*- coding: UTF-8 -*-
import sys
from aip import AipSpeech

reload(sys)
sys.setdefaultencoding('utf-8')

APP_ID = '10205809'
API_KEY = 'W8HqNvw0BosgXd0eoxQeQxeq'
SECRET_KEY = 'CtEyonALjCrmgbLHAQQL7dKNEtrqH4WQ'

aipSpeech = AipSpeech(APP_ID, API_KEY, SECRET_KEY)

result  = aipSpeech.synthesis('好的，知道了', 'zh', 1, {
    'vol': 5,
})
#print result
if not isinstance(result, dict):
    with open('auido.mp3', 'wb') as f:
        f.write(result)
