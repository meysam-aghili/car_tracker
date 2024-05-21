#ifndef mqtt_h
#define mqtt_h

#include <PubSubClient.h>
#include "gsm.h"
#include "util.h"
#include "config.h"

PubSubClient mqtt_client(connection_client);
bool mqtt_is_healthy = false;

void mqtt_set_health_state(bool state)
{
  mqtt_is_healthy = state;
  (state == true) ? digitalWrite(HLTMQTT, HIGH) : digitalWrite(HLTMQTT, LOW);
}

bool check_connection_mqtt_broker()
{
  if (!gsm_check_gprs())
  {
    mqtt_set_health_state(false);
    return false;
  }
  if (!mqtt_client.connected())
  {
    log_print("mqtt not connected");
  }
  unsigned long start_time = millis();
  while (!mqtt_client.connected() && (millis() - start_time) <= 10000)
  {
    if (mqtt_client.connect(broker_client_id.c_str(), broker_username.c_str(), broker_password.c_str()))
    {
      log_print("MQTT broker connected");
    }
    else
    {
      log_print("failed with state " + String(mqtt_client.state()));
      mqtt_set_health_state(false);
      return false;
    }
  }
  if (mqtt_client.connected())
  {
    mqtt_set_health_state(true);
    return true;
  }
  else
  {
    mqtt_set_health_state(false);
    return false;
  }
}

bool mqtt_init()
{
  pinMode(HLTMQTT, OUTPUT);
  digitalWrite(HLTMQTT, LOW);
  mqtt_client.setServer(broker_ip.c_str(), broker_port);
  return check_connection_mqtt_broker();
}

bool publish_to_mqtt_broker(const char *topic, const char *payload)
{
  if (mqtt_client.publish(topic, payload))
  {
    return true;
  }
  else
  {
    check_connection_mqtt_broker();
    return mqtt_client.publish(topic, payload);
  }
}

#endif