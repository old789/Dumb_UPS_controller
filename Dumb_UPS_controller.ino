#define DEBUG_SERIAL  // because just "DEBUG" defined in PZEM004Tv30.h ( legacy :)
#define DBG_WIFI    // because "DEBUG_WIFI" defined in a WiFiClient library

#if defined ( DBG_WIFI ) && not defined ( DEBUG_SERIAL )
#define DEBUG_SERIAL
#endif

// #define UPS

#if not defined UPS
#define POWER_SENSOR
#endif

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "TickTwo.h"    // https://github.com/sstaub/TickTwo

#include "uptime.h"     // https://github.com/YiannisBourkelis/Uptime-Library


#define PIN_EXT_POWER   3 // GPIO3 (RX)
#if defined UPS
#define PIN_LOW_BATT    2 // GPIO2
#endif
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
#if defined ( UPS )
byte battery_state = HIGH;
byte battery_state_prev = HIGH;
#endif
bool first_report = true;
int httpResponseCode = 0;

#include "config.h"

char str_uptime[17] = "0d0h0m0s";
char str_post[1024];


void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  delay(50);
  Serial.println(".\nStart debugging serial");
#else
  pinMode(PIN_LED, OUTPUT);
#endif

  pinMode(PIN_EXT_POWER, INPUT);
#if defined ( UPS )
  pinMode(PIN_LOW_BATT,  INPUT);
#endif

  wifi_init();

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
#if defined ( DEBUG_SERIAL )
  PGM_P msg_pwr_fail = PSTR("External power failed");
  PGM_P msg_pwr_restore = PSTR("External power restored");
  PGM_P msg_battery_low = PSTR("Low battery");
  PGM_P msg_battery_ok = PSTR("Battery is Ok");
#else
  digitalWrite(PIN_LED, digitalRead(PIN_LED) ^ 1);
#endif

  // read and logging external power state
  external_power_state = digitalRead(PIN_EXT_POWER);   
  if (external_power_state_prev != external_power_state) {
    external_power_state_prev = external_power_state;
    if (external_power_state == LOW) {
      send_alarm_ab_input( false );
#ifdef DEBUG_SERIAL
      Serial.println(FPSTR(msg_pwr_fail));
#endif
    } else {
      send_alarm_ab_input( true );
#ifdef DEBUG_SERIAL
      Serial.println(FPSTR(msg_pwr_restore));
#endif
    }
  }

#if defined ( UPS )
  battery_state = digitalRead(PIN_LOW_BATT);   
  if (battery_state_prev != battery_state) {
    battery_state_prev = battery_state;
    if (battery_state == LOW) {
      send_alarm_ab_battery( false );
#ifdef DEBUG_SERIAL
      Serial.println(FPSTR(msg_battery_low));
#endif
    } else {
      send_alarm_ab_battery( true );
#ifdef DEBUG_SERIAL
      Serial.println(FPSTR(msg_battery_ok));
#endif
    }
  }
#endif

}

void count_uptime() {
  uptime::calculateUptime();
  memset(str_uptime, 0, sizeof(str_uptime));
  sprintf( str_uptime, "%ud%uh%um%us", uptime::getDays(), uptime::getHours(), uptime::getMinutes(), uptime::getSeconds() );
}
