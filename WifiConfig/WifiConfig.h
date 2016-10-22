#ifndef WifiConfig_H
#define WifiConfig_H

#include <ESP8266WebServer.h>
#include <WString.h>
#define AP_RESTART "restart"
#define AP_CLEAREEPROM "cleareeprom"
#define AP_INFO "info"
#define AP_WIFICFGPORT 80
#define ESPWIFI2EEPROM_VERSION "0.5"

// password for the esp8266 softAP creates when it doesn't find info in the
// eeprom
// or when something else goes wrong and can't connect
const char AP_password[] = "";

class WifiConfig {
public:
	WifiConfig();
	void initBasicHttpServer();

	ESP8266WebServer server;
	void espNKWiFiconnect();
// private:
	ESP8266WebServer APserver;
	bool testWiFi();
	String printConnectionType(int thisType);
	void setupWiFiAP();
	void getAPlist();
	String printEncryptionType(int thisType);

	//AP web server handlers
	void handle_AProot();
	void handle_APsubmit();
	void handle_APrestart();
	void handle_clearAPeeprom();

	void handle_root();
};
#endif
