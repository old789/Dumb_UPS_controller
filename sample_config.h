//   Config begin

// Name of system for logging
char ups_name[33] = {0};

// Model of device
char ups_model[33] = {0};

// WiFi SSID
char ssid[33] = {0};

// WiFi password
char passw[65] = {0};

// hostname for logging
char host[65] = {0};

// remote port 
uint16_t port = 443;

// URI of the script that writes to the log
char uri[128] = {0};

// Enable HTTP auth ( 1 - enable )
uint8_t http_auth = 0;

// HTTP user and password for auth
char http_user[33] = {0};
char http_passw[33] = {0};

//   Config end
