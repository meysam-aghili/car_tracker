import paho.mqtt.client as mqtt
import time


def on_connect(client, userdata, flags, return_code):
    if return_code == 0:
        print("connected")
    else:
        print("could not connect, return code:", return_code)

client = mqtt.Client("ClientPub")
client.username_pw_set(username="meysam", password="meysam")
client.on_connect=on_connect
client.connect('akrana.ddns.net', 1883)

topic = "test"
msg_count = 0

try:
    while msg_count < 40:
        time.sleep(1)
        msg_count += 1
        result = client.publish(topic, 'msg_'+str(msg_count))
        status = result[0]
        if status == 0:
            print("Message "+ 'msg_'+str(msg_count) + " is published to topic " + topic)
        else:
            print("Failed to send message to topic " + topic)
finally:
    client.loop_stop()