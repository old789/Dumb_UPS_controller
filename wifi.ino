void make_post_header(){
  memset(str_post,0,sizeof(str_post));
  strncpy(str_post, "uptime=", sizeof(str_post)-1);
  strncat(str_post, str_uptime, sizeof(str_post)-1);
  
  if (strlen(ups_name) > 0) {
    strncat(str_post, "&name=", sizeof(str_post)-1);
    strncat(str_post, ups_name, sizeof(str_post)-1);
  } 
      
  if ( first_report ) {
    strncat(str_post, "&boot=1", sizeof(str_post)-1);
  }
}  

void send_alarm_ab_input( bool wtf ){
  make_post_header();
  if ( wtf ) {
    strncat(str_post, "&alarm=power restored", sizeof(str_post)-1);
  } else {
    strncat(str_post, "&alarm=no input voltage", sizeof(str_post)-1);
  }

#ifdef DBG_WIFI
  Serial.print("Alarm: \""); Serial.print(str_post); Serial.println("\"");
#endif
  send_data();
}

void send_alarm_ab_battery( bool wtf ) {
  make_post_header();
  if ( wtf ) {
    strncat(str_post, "&alarm=battery is Ok", sizeof(str_post)-1);
  } else {
    strncat(str_post, "&alarm=low battery", sizeof(str_post)-1);
  }
  
#ifdef DBG_WIFI
  Serial.print("Alarm: \""); Serial.print(str_post); Serial.println("\"");
#endif
  send_data();
}

void usual_report(){
  char str_batt[12] = {0};
  char str_power[10] = {0};
  char str_tmp[128];
 
  if ( external_power_state == HIGH ) {
    strncpy(str_power, "powerOk", sizeof(str_power)-1);
  } else {
    strncpy(str_power, "nopower", sizeof(str_power)-1);
  }
 
  if ( battery_state == HIGH ) {
    strncpy(str_batt, "batteryOk", sizeof(str_batt)-1);
  } else {
    strncpy(str_batt, "batteryLow", sizeof(str_batt)-1);
  }
  
  sprintf(str_tmp, "&data1=%s,%s,%s,%d", str_power, str_batt, WiFi.localIP(), WiFi.RSSI() );
  
  make_post_header();
  strncat(str_post, str_tmp, sizeof(str_post)-1);

#ifdef DBG_WIFI
  Serial.print("Prepared data: \""); Serial.print(str_post); Serial.println("\"");
#endif
  send_data();
}

void send_data(){

  if ( strlen(str_post) == 0 ){
#ifdef DBG_WIFI
    Serial.println("Nothing to send");
#endif
    return;
  }

  //Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED){
#ifdef DBG_WIFI
    Serial.println("WiFi Disconnected");
#endif
    wifi_init();
  }
  
#ifdef DBG_WIFI
  Serial.println("Send data");
#endif

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  // Ignore SSL certificate validation
  client->setInsecure();
  
  //Serial.println("HTTP client");
  HTTPClient http;

  //Serial.println("http begin");
  // Your Domain name with URL path or IP address with path
  // http.begin(client, serverName);
  http.begin(*client, host, port, uri);

  if ( http_auth > 0 ) {
    http.setAuthorization(http_user, http_passw);
  }
  
  //Serial.println("http header");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  //http.addHeader("Content-Type", "text/plain");
  //Serial.println("http post");
  httpResponseCode = http.POST(str_post);

#ifdef DBG_WIFI
  Serial.print("HTTP Response code: "); Serial.println(httpResponseCode);
#endif

  // Free resources
  http.end();
  
  if ( httpResponseCode == 200 ) {
    if ( first_report ) {
      first_report = false;
    }
  }
}

void wifi_init(){
#ifdef DBG_WIFI
  Serial.print("Connecting to "); Serial.print(ssid); Serial.println(" ...");
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passw);             // Connect to the network

  uint16_t i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(100);
    i++;
#ifdef DBG_WIFI
    Serial.print(i); Serial.print(' ');
#endif
    if ( i > 1500 ) {  // if don't connect - wait, then next try
      if ( ++wifi_tries > 3 ) {
        ESP.reset();
      } 
      delay(150000);
#ifdef DBG_WIFI
      Serial.print("Try "); Serial.print(wifi_tries); Serial.println(" connect to WiFi");
#endif
    }
  }
  
  if ( wifi_tries != 0 ) {
    wifi_tries=0;
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

#ifdef DBG_WIFI
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address: ");Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");Serial.println(WiFi.RSSI());
#endif
}
