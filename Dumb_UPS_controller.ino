#define DEBUG_SERIAL  // because just "DEBUG" defined in PZEM004Tv30.h ( legacy :)
#define DBG_WIFI    // because "DEBUG_WIFI" defined in a WiFiClient library

#if defined ( DBG_WIFI ) && not defined ( DEBUG_SERIAL )
#define DEBUG_SERIAL
#endif

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "TickTwo.h"    // https://github.com/sstaub/TickTwo

#include "uptime.h"     // https://github.com/YiannisBourkelis/Uptime-Library


#define PIN_EXT_POWER   2 // GPIO2
#define PIN_LOW_BATT    3 // GPIO3 (RX)
#if not defined ( DEBUG_SERIAL )
#define PIN_LED         1 // GPIO1 (TX);
#endif

void count_uptime();
void usual_report();
void check_ups_status();

// Create timers object
TickTwo timer1( count_uptime, 1000);
TickTwo timer2( check_ups_status, 2000);
TickTwo timer3( usual_report, 60000);

byte external_power_state = HIGH;
byte external_power_state_prev = HIGH;
unsigned int last_change_state = 0;
uint8_t wifi_tries = 0;
bool first_report = true;
int httpResponseCode = 0;

//   Config begin
char ups_name[33] = {0};
char ssid[33] = {0};
char passw[65] = {0};
char host[65] = {0};
uint16_t port = 443;
char uri[128] = {0};
uint8_t http_auth = 0;
char http_user[33] = {0};
char http_passw[33] = {0};
//   Config end

char str_uptime[17] = "0d0h0m0s";
char str_post[1024];


void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  delay(50);
  Serial.println(".\nStart debugging serial");
#endif

  pinMode(PIN_EXT_POWER, INPUT);
  pinMode(PIN_LOW_BATT,  INPUT);

  timer1.start();
  timer2.start();
  timer3.start();
}

void loop() {
  timer1.update();
  timer2.update();
  timer3.update();
}

void check_ups_status(){
#if defined ( USE_SERIAL )
  PGM_P msg_pwr_fail = PSTR("Ext.power failed");
  PGM_P msg_pwr_restore = PSTR("Ext.power restored");
#endif

  external_power_state = digitalRead(PIN_EXT_POWER);   // read and logging external power state
  if (external_power_state_prev == external_power_state) {
    last_change_state++;
  } else {
    last_change_state = 0;
    external_power_state_prev = external_power_state;
#ifdef USE_SERIAL
    if (external_power_state == LOW) Serial.println(FPSTR(msg_pwr_fail));
    else Serial.println(FPSTR(msg_pwr_restore));
#endif
  }

}

void count_uptime() {
  uptime::calculateUptime();
  memset(str_uptime, 0, sizeof(str_uptime));
  sprintf( str_uptime, "%ud%uh%um%us", uptime::getDays(), uptime::getHours(), uptime::getMinutes(), uptime::getSeconds() );
}
