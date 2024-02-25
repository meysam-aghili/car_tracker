#ifndef mqtt_h
#define mqtt_h

#include <PubSubClient.h>
class mqtt
{
public:
  String broker;
  uint16_t port;
  String client_id;
  String user;
  String pass;
  PubSubClient mqtt_client;
  mqtt(String broker_ip, uint16_t broker_port, String broker_client_id, String broker_username, String broker_password, PubSubClient &mqtt_client_)
  {
    this->broker = broker_ip;
    this->port = broker_port;
    this->client_id = broker_client_id;
    this->user = broker_username;
    this->pass = broker_password;
    this->mqtt_client = mqtt_client_;
  }
  void setup_mqtt_broker()
  {
    mqtt_client.setServer(this->broker.c_str(), this->port);
  }
  bool check_connection_mqtt_broker()
  {
    if (!mqtt_client.connected())
    {
      Serial.println("mqtt broker not connected.");
      Serial.print("connecting to ");
      Serial.println(broker);
      unsigned long start_time = millis();
      while (!mqtt_client.connected() && (millis() - start_time) <= 10000)
      {
        mqtt_client.connect(this->client_id.c_str(), this->user.c_str(), this->pass.c_str());
        delay(500);
      }
      if (!mqtt_client.connected())
      {
        Serial.println("mqtt broker connection attempt failed");
        delay(5000);
        return 0;
      }
      else
      {
        Serial.println("mqtt broker connected");
        delay(100);
        return 1;
      }
    }
    else
    {
      return 1;
    }
  }
  bool publish_to_mqtt_broker(const char *topic, const char *payload)
  {
    if (check_connection_mqtt_broker())
    {
      return mqtt_client.publish(topic, payload);
    }
    else
    {
      return 0;
    }
  }
  void mqtt_loop()
  {
    mqtt_client.loop();
  }
};

#endif