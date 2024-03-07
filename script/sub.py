import paho.mqtt.client as mqtt
import time


def on_message(client, userdata, message):
    print("Received message: ", str(message.payload.decode("utf-8")))

def on_connect(client, userdata, flags, return_code):
    if return_code == 0:
        print("connected")
        client.subscribe("gps")
    else:
        print("could not connect, return code:", return_code)

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1,"ClientSub")
client.username_pw_set(username="admin", password="admin")
client.on_connect=on_connect
client.on_message=on_message
client.connect('localhost', 1883)
client.loop_start()

try:
    time.sleep(100)
finally:
    client.loop_stop()