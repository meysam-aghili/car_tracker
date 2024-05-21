#ifndef gsm_h
#define gsm_h

#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#include "util.h"
#include "config.h"

SoftwareSerial gsm_serial(RXGSM, TXGSM);
TinyGsm gsm(gsm_serial);
TinyGsmClient connection_client(gsm);
bool gsm_is_healthy = false;
bool gsm_network_is_healthy = false;
bool gsm_gprs_is_healthy = false;

void gsm_set_health_state(bool state)
{
  gsm_gprs_is_healthy = state;
  (state == true) ? digitalWrite(HLTGSM, HIGH) : digitalWrite(HLTGSM, LOW);
}

void gsm_power_on()
{
  digitalWrite(PWRGSM,HIGH);  
}

void gsm_power_off()
{
  digitalWrite(PWRGSM,LOW);  
}

void gsm_hard_restart()
{
  gsm_power_off();
  delay(100);
  gsm_power_on();
}

void send_at(String cmd)
{
  gsm_serial.listen();
  log_print("send : " + cmd);
  gsm_serial.println(cmd);
  gsm_serial.flush();
}

int8_t wait_response(uint32_t timeout_ms, String &data)
{
  gsm_serial.listen();
  uint32_t startMillis = millis();
  data.reserve(64);
  do
  {
    while (gsm_serial.available())
    {
      int8_t a = gsm_serial.read();
      if (a <= 0)
        continue; // Skip 0x00 bytes, just in case
      data += static_cast<char>(a);
      if (data.endsWith("OK\r\n"))
      {
        log_print("ok");
        return 1;
      }
      else if (data.endsWith("ERROR\r\n"))
      {
        log_print("error");
        return 2;
      }
    }
  } while (millis() - startMillis < timeout_ms);
  log_print("error : not responsed");
  return 0;
}

int8_t wait_response(uint32_t timeout_ms)
{
  String data;
  return wait_response(timeout_ms, data);
}

int8_t wait_response()
{
  return wait_response(1000);
}

bool gsm_check_heartbeat()
{
  unsigned int restart_counter = 2;
  for (size_t i = 0; i < restart_counter; i++)
  {
    unsigned int at_counter = 0;
    while (at_counter < 10)
    {
      send_at("AT");
      if (wait_response(1000) == 1)
      {
        gsm_is_healthy = true;
        return true;
      }
      at_counter++;
    }
    gsm_hard_restart();
  }
  gsm_is_healthy = false;
  return false;
}

bool gsm_init()
{
  gsm_serial.begin(115200);
  pinMode(PWRGSM ,OUTPUT);
  pinMode(HLTGSM, OUTPUT);
  digitalWrite(HLTGSM, LOW);
  gsm_hard_restart();
  delay(50);
  gsm_serial.listen();
  if (gsm_check_heartbeat() == false)
  {
    return false;
  }

  send_at("ATE0");
  if (wait_response(10) != 1)
  {
    return false;
  }

  send_at("AT+CMEE=0"); // turn off error codes
  wait_response();

  send_at("AT+CLTS=1"); // Enable Local Time Stamp for getting network time
  if (wait_response(10000L) != 1)
  {
    return false;
  }

  //! this will reset module every time
  // send_at("AT+CBATCHK=1"); // Enable battery checks
  // wait_response();

  return true;
}

bool gsm_check_network(uint32_t timeout_ms = 60000L)
{
  if (gsm_check_heartbeat())
  {
    for (uint32_t start = millis(); millis() - start < timeout_ms;)
    {
      send_at("AT+CREG?");
      String data;
      if (wait_response(10, data) == 1)
      {
        int status_index = data.indexOf(",");
        String status = data.substring(status_index + 1, status_index + 2);
        log_print("network status: " + status);
        // 0 Not registered, the device is currently not searching for new operator.
        // 1 Registered to home network.
        // 2 Not registered, but the device is currently searching for a new operator.
        // 3 Registration denied.
        // 4 Unknown. For example, out of range.
        // 5 Registered, roaming. The device is registered on a foreign (national or international) network.
        if (status == "1" || status == "5")
        {
          gsm_network_is_healthy = true;
          return true;
        }
      }
      delay(2000L);
    }
  }
  gsm_network_is_healthy = false;
  return false;
}

bool gsm_gprs_disconnect() 
{
    // Shut the TCP/IP connection
    // CIPSHUT will close *all* open connections
    send_at("AT+CIPSHUT");
    if (wait_response(60000L) != 1) { return false; }

    send_at("AT+CGATT=0");  // Detach from GPRS
    if (wait_response(60000L) != 1) { return false; }

    return true;
  }

bool gsm_gprs_connect(String apn, String user = "", String pwd = "")
{
  gsm_gprs_disconnect();

  send_at("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  wait_response();

  send_at("AT+SAPBR=3,1,\"APN\",\"" + apn + '"');
  wait_response();

  if (user && user.length() > 0)
  {
    send_at("AT+SAPBR=3,1,\"USER\",\"" + user + '"');
    wait_response();
  }
  if (pwd && pwd.length() > 0)
  {
    send_at("AT+SAPBR=3,1,\"PWD\",\"" + pwd + '"');
    wait_response();
  }

  // Define the PDP context
  send_at("AT+CGDCONT=1,\"IP\",\"" + apn + '"');
  wait_response();

  // Activate the PDP context
  send_at("AT+CGACT=1,1");
  wait_response(60000L);

  // Open the definied GPRS bearer context
  send_at("AT+SAPBR=1,1");
  wait_response(85000L);
  // Query the GPRS bearer context status
  send_at("AT+SAPBR=2,1");
  if (wait_response(30000L) != 1)
  {
    return false;
  }

  // Attach to GPRS
  send_at("AT+CGATT=1");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Set to multi-IP
  send_at("AT+CIPMUX=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Put in "quick send" mode (thus no extra "Send OK")
  send_at("AT+CIPQSEND=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Set to get data manually
  send_at("AT+CIPRXGET=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Start Task and Set APN, USER NAME, PASSWORD
  send_at("AT+CSTT=\"" + apn + "AT\",\"" + user + "AT\",\"" + pwd + "AT\"");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Bring Up Wireless Connection with GPRS or CSD
  send_at("AT+CIICR");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Get Local IP Address, only assigned after connection
  send_at("AT+CIFSR;E0");
  if (wait_response(10000L) != 1)
  {
    return false;
  }

  // Configure Domain Name Server (DNS)
  send_at("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"");
  if (wait_response() != 1)
  {
    return false;
  }

  return true;
}

bool gsm_check_gprs()
{
  if (!gsm_check_network())
  {
    gsm_set_health_state(false);
    return false;
  }
  send_at("AT+SAPBR=2,1");
  String data;
  if (wait_response(10, data) == 1)
  {
    int status_index = data.indexOf("0.0.0.0");
    log_print("gprs status: " + String(status_index));
    //-1 connected.
    //else not connected. 
    if (status_index == -1)
    {
      gsm_set_health_state(true);
      return true;
    }
    else
    {
      gsm_set_health_state(gsm_gprs_connect(apn_name));
      return gsm_gprs_is_healthy;
    }
  }
  gsm_set_health_state(false);
  return false;
}

#endif