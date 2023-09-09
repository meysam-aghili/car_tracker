import paho.mqtt.client as mqtt
import time
import datetime

def on_connect(client, userdata, flags, return_code):
    if return_code == 0:
        print("connected")
    else:
        print("could not connect, return code:", return_code)

client = mqtt.Client("ClientPub")
client.username_pw_set(username="meysam", password="meysam")
client.on_connect=on_connect
client.connect('localhost', 1883)

topic = "test"
msg_count = 0

try:
    while msg_count < 40:
        time.sleep(2)
        msg_count += 1
        idd = str(int(datetime.datetime.now().timestamp()))+"_1"
        result = client.publish(topic, '{'+'"device_id":1,"datetime":"2023-09-06 16:03:46","latitude":35.75318883,"longitude":51.19973483,"satellites":0,"altitude":0,"speed":0.31,"course":0,"hdop":0'+'}')
        status = result[0]
        if status == 0:
            print("Message is published to topic " + topic)
        else:
            print("Failed to send message to topic " + topic)
finally:
    client.loop_stop()