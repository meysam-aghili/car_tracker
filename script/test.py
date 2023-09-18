import datetime

while True:
    idd = str(int(datetime.datetime.now().timestamp()))+'_1'
    print('{'+'"id":"{0}","device_id":"1","datetime":"2023-09-06 16:03:46","latitude":35.75318883,"longitude":51.19973483,"satellites":0,"altitude":0,"speed":0.31,"course":0,"hdop":0}'.format(idd)+'}')

