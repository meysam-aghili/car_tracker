#ifndef gsm_h
#define gsm_h

#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
class connection_gsm
{
public:
  String apn;
  String user;
  String pass;
  String pin_code;
  TinyGsm *gsm;
  connection_gsm(String apn_name, String username, String password, String sim_pin_code, TinyGsm *gsm_)
  {
    this->apn = apn_name;
    this->user = username;
    this->pass = password;
    this->pin_code = sim_pin_code;
    this->gsm = gsm_;
  }
  bool check_connection_gsm()
  {
    if (!this->gsm->isNetworkConnected())
    {
      Serial.println("no network");
      Serial.println("connecting to gsm network...");

      if (!this->gsm->waitForNetwork(180000L))
      {
        Serial.println("gsm network failed");
        delay(5000);
        return 0;
      }
      else
      {
        Serial.println("gsm network connected");
        return 1;
      }
    }
    else
    {
      return 1;
    }
  }
  bool check_connection_gprs()
  {
    if (!this->gsm->isGprsConnected())
    {
      Serial.println("no gprs network");
      Serial.println("connecting to ");
      Serial.println(this->apn);
      bool status = this->gsm->gprsConnect(this->apn.c_str(), this->user.c_str(), this->pass.c_str());
      if (!status)
      {
        Serial.println("gprs connection attempt failed");
        delay(10000);
        return 0;
      }
      else
      {
        Serial.println("gprs connected");
        return 1;
      }
    }
    else
    {
      return 1;
    }
  }
  bool check_connection()
  {
    if (check_connection_gsm())
    {
      return check_connection_gprs();
    }
    else
    {
      return 0;
    }
  }
  bool setup_connection()
  {
    Serial.println("initializing modem");
    this->gsm->init();
    Serial.print("modem info: ");
    Serial.println(this->gsm->getModemInfo());
    Serial.print("sim status: ");
    Serial.println(this->gsm->getSimStatus());
    if (this->gsm->getSimStatus() == 3)
    {
      this->gsm->simUnlock(this->pin_code.c_str());
    }
    return check_connection();
  }
};

#endif