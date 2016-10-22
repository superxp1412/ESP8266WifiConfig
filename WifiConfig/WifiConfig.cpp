#include "WifiConfig.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// defined in espWiFi2eeprom.h , keywords to restart or clear eeprom
const char *restartcommand = "/" AP_RESTART;
const char *cleareepromcommand = "/" AP_CLEAREEPROM;
const char *infocommand = "/" AP_INFO;

// web page parts
const char APwebPage1[] PROGMEM = "<!DOCTYPE HTML>\n"
				  "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>esp8266 WiFi setup control</title>\n"
				  "<style type=\"text/css\">body {text-align:center;font-family: sans-serif;background-color: #000;color: #fff;font-size:1.2em;} a:link, a:visited, a:hover, a:active {color:#fff;}</style>\n"
				  "</head>\n"
				  "<body>\n"
				  "<h1>esp8266 WiFi setup control</h1>\n<br>"
				  "<table style=\"width:100%;border: 1px solid #fff;\"><tr>"
				  "<th style=\"text-align:center;width:50%;\"><form action='/APsubmit' method='POST'><input type=\"text\" name=\"newssid\" id=\"formnewssid\" value=\"\"><br><input type=\"text\" name=\"newpass\" value=\"\" size=\"32\" maxlength=\"64\"><br><input type=\"submit\" value=\"Submit\"></form></th>"
				  "<th style=\"text-align:left;width:50%;\">";
String APwebPage2 = "</th></tr></table>\n"
		    "<br><br><form action=\"/\" target=\"_top\"><input type=\"submit\" value=\"home / rescan networks\"></form>\n"
		    "<br><br><form action=\"" + String(restartcommand) + "\" target=\"_top\"><input type=\"submit\" value=\"restart esp8266\"></form>\n"
		    "<br><br><form action=\"" + String(cleareepromcommand) + "\" target=\"_top\"><input type=\"submit\" value=\"! clear EEPROM !\"></form>\n"
		    "<br><br><b>- version: " ESPWIFI2EEPROM_VERSION " -</b>\n"
		    "</body></html>";

String webPage1 = "<!DOCTYPE HTML>\n"
                  "<html><head><meta content=\"text/html;charset=utf-8\"><title>espWiFi2eeprom esp8266 example page</title>\n"
                  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
                  "</head>\n"
                  "<body>\n"
                  "<center>\n"
                  "<h1>espWiFi2eeprom</h1>"
                  "<h2>esp8266 example page</h2>\n";

String webPage2 =  "<br><hr>\n"
                   "</center>\n"
                   "</body></html>";

String APwebstring = "";   // String to display

WifiConfig::WifiConfig()
{
	server = ESP8266WebServer(80);
	APserver = ESP8266WebServer(AP_WIFICFGPORT);
}

// when the form with ssid and pass is submited write them to eeprom
void WifiConfig::handle_APsubmit()
{
	String thenewssid = APserver.arg("newssid");
	String thenewpass = APserver.arg("newpass");

	if (thenewssid != "") {
		Serial.println(F("! Clearing eeprom !"));
		for (int i = 0; i < 96; ++i)
			EEPROM.write(i, 0);

		Serial.println(F("Writing SSID to EEPROM"));
		for (int i = 0; i < thenewssid.length(); ++i) {
			EEPROM.write(i, thenewssid[i]);
			//Serial.print("Wrote: ");
			//Serial.println(thenewssid[i]);
		}
		Serial.println(F("Writing password to EEPROM"));
		for (int i = 0; i < thenewpass.length(); ++i) {
			EEPROM.write(32 + i, thenewpass[i]);
			//Serial.print("Wrote: ");
			//Serial.println(thenewpass[i]);
		}

		if (EEPROM.commit())
			APwebstring = F("<h2><b><p>Saved to eeprom... restart to boot into new wifi</b></h2>\n");
		else
			APwebstring = F("<h2><b><p>Couldn't write to eeprom. Please try again.</b></h2>\n");
		delay(10);
		String SServerSend;
		SServerSend = FPSTR(APwebPage1);
		SServerSend += APwebstring + APwebPage2;
		APserver.send(200, "text/html", SServerSend);
		delay(100);
	}
}

// restart esp8266
void WifiConfig::handle_APrestart()
{
	Serial.println(F("! Restarting in 1 sec! !"));
	delay(1000);
	ESP.restart();
}

// return the connection type for the AP list
String WifiConfig::printEncryptionType(int thisType)
{
	String enc_type = "";

	// read the encryption type and print out the name:
	switch (thisType) {
	case ENC_TYPE_WEP:
		return enc_type = "WEP";
	case ENC_TYPE_TKIP:
		return enc_type = "WPA";
	case ENC_TYPE_CCMP:
		return enc_type = "WPA2";
	case ENC_TYPE_NONE:
		return enc_type = "None";
	case ENC_TYPE_AUTO:
		return enc_type = "Auto";
	default:
		return enc_type = "?";
	}
}


// responds to local server / call
void WifiConfig::handle_AProot()
{
	getAPlist();
	String SServerSend;
	SServerSend = FPSTR(APwebPage1);
	SServerSend += APwebstring + APwebPage2;
	APserver.send(200, "text/html", SServerSend);
	delay(100);
}


// clear first bytes of eeprom
void WifiConfig::handle_clearAPeeprom()
{
	Serial.println(F("! Clearing eeprom !"));
	for (int i = 0; i < 96; ++i)
		EEPROM.write(i, 0);
	EEPROM.commit();
	delay(1000);
	ESP.restart();
}

/***
 * /* Public methods
 */

// get available AP to connect + HTML list to display
void WifiConfig::getAPlist()
{
	WiFi.disconnect();
	delay(100);
	int n = WiFi.scanNetworks();
	Serial.println(F("Scan done"));
	if (n == 0) {
		Serial.println(F("No networks found :("));
		APwebstring = F("No networks found :(");
		return;
	}

	// sort by RSSI
	int indices[n];
	for (int i = 0; i < n; i++)
		indices[i] = i;
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++)
			if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
				std::swap(indices[i], indices[j]);
	}

	Serial.println("");
	// HTML Print SSID and RSSI for each network found
	APwebstring = F("<ul>");
	for (int i = 0; i < n; ++i) {
		APwebstring += F("<li>");
		APwebstring += i + 1;
		APwebstring += F(":&nbsp;&nbsp;<b>");
		APwebstring += F("<a href=\"#\" target=\"_top\" onClick=\"document.getElementById(\'formnewssid\').value=\'");
		APwebstring += WiFi.SSID(indices[i]);
		APwebstring += F("\'\">");
		APwebstring += WiFi.SSID(indices[i]);
		APwebstring += F("</a>");
		APwebstring += F("</b>&nbsp;&nbsp;&nbsp;(");
		APwebstring += WiFi.RSSI(indices[i]);
		APwebstring += F("&nbsp;dBm)&nbsp;&nbsp;&nbsp;");
		APwebstring += printEncryptionType(WiFi.encryptionType(indices[i]));
		APwebstring += F("</li>");
	}
	APwebstring += F("</ul>");
	delay(100);
}

// setup a soft AP for the user to connect to esp8266 and give a new ssid and pass
void WifiConfig::setupWiFiAP()
{
	WiFi.mode(WIFI_AP);// wifi mode can connect to AP
	
	// Do a little work to get a unique-ish name for the soft AP. Append the
	// last two bytes of the MAC (HEX'd):
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.softAPmacAddress(mac);
	String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
		       String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
	macID.toUpperCase();
	String AP_NameString = F("esp8266 control ");
	AP_NameString += macID;

	char AP_NameChar[AP_NameString.length() + 1];
	memset(AP_NameChar, 0, AP_NameString.length() + 1);

	for (int i = 0; i < AP_NameString.length(); i++)
		AP_NameChar[i] = AP_NameString.charAt(i);

	getAPlist();

	WiFi.softAP(AP_NameChar, AP_password);

	APserver.on("/", std::bind(&WifiConfig::handle_AProot, this));
	APserver.on("/APsubmit", std::bind(&WifiConfig::handle_APsubmit, this));
	APserver.on(restartcommand, std::bind(&WifiConfig::handle_APrestart, this));
	APserver.on(cleareepromcommand, std::bind(&WifiConfig::handle_clearAPeeprom, this));
	APserver.begin();

	Serial.print(F("SoftAP IP address: "));
	Serial.println(WiFi.softAPIP());

	while (WiFi.status() != WL_CONNECTED)
		APserver.handleClient();
}

void WifiConfig::espNKWiFiconnect()
{
	WiFi.mode(WIFI_STA);

	EEPROM.begin(512);
	Serial.println("");
	// read eeprom for ssid and pass
	Serial.println(F("Reading EEPROM SSID"));
	String esid = "";
	for (int i = 0; i < 32; ++i)
		esid += char(EEPROM.read(i));
	//Serial.print("SSID: ");
	//Serial.println(esid);
	Serial.println(F("Reading EEPROM password"));
	String epass = "";
	for (int i = 32; i < 96; ++i)
		epass += char(EEPROM.read(i));
	//Serial.print("PASS: ");
	//Serial.println(epass);

	// if ssid not empty try to connect
	if (esid != "") {
		// test esid
		WiFi.begin(esid.c_str(), epass.c_str());

		boolean testWiFiAP = testWiFi();

		if (testWiFiAP) {
			return;
		} else if (!testWiFiAP) {
			Serial.println(F("Could not connect to SSID!"));
			if (WiFi.status() == WL_CONNECT_FAILED) {
				// we found the AP but connection failed. maybe pass is wrong.
				// clear eeprom and restart esp so we can try again
				Serial.println(F("Connection failed! Maybe SSID/password are wrong."));
				Serial.println(F("Clearing EEPROM and restarting!"));
				delay(500);
				handle_clearAPeeprom();
				delay(100);
				handle_APrestart();
			} else {
				// could not connect to ssid so starting soft AP
				Serial.println("Starting AP at port: " + String(AP_WIFICFGPORT));
				setupWiFiAP();
			}
		}
	} else {
		// ssid not found in eeprom so starting soft AP
		Serial.println("SSID empty! Starting AP at port: " + String(AP_WIFICFGPORT));
		setupWiFiAP();
	}
}

// return the connection type for the AP list
String WifiConfig::printConnectionType(int thisType)
{
	String con_type = "";

	// read connection type and print out the name:
	switch (thisType) {
	case 255:
		return con_type = "WL_NO_SHIELD";
	case 0:
		return con_type = "WL_IDLE_STATUS";
	case 1:
		return con_type = "WL_NO_SSID_AVAIL";
	case 2:
		return con_type = "WL_SCAN_COMPLETED";
	case 3:
		return con_type = "WL_CONNECTED";
	case 4:
		return con_type = "WL_CONNECT_FAILED";
	case 5:
		return con_type = "WL_CONNECTION_LOST";
	case 6:
		return con_type = "WL_DISCONNECTED";
	default:
		return con_type = "?";
	}
}

// test if we are connected
bool WifiConfig::testWiFi()
{
	int c = 0;

	Serial.print(F("Waiting for Wifi to connect"));
	// c at 60 with delay 500 is for 30 seconds ;)
	while (c < 60) {
		Serial.print(".");
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("");
			Serial.println(printConnectionType(WiFi.status()));
			return 1;
		}
		delay(500);
		c++;
	}
	Serial.println("");
	Serial.println(printConnectionType(WiFi.status()));
	return 0;
}

void WifiConfig::handle_root() {
	String webString = "";   // String to display

  // just something to output to webpage
  webString = "ESP Chip ID: " + String(ESP.getChipId()) + "<br>ESP Flash Chip ID: " + String(ESP.getFlashChipId()) + "<br>ESP Flash Chip Size: " + String(ESP.getFlashChipSize()) + "<br>ESP Free Heap: " + String(ESP.getFreeHeap());
  server.send(200, "text/html", webPage1 + webString + webPage2);
  delay(100);
}

void WifiConfig::initBasicHttpServer(){
	server.on(restartcommand, std::bind(&WifiConfig::handle_APrestart, this));
	server.on(cleareepromcommand, std::bind(&WifiConfig::handle_clearAPeeprom, this));
	server.on(infocommand, std::bind(&WifiConfig::handle_root, this));
	server.begin();
	Serial.println("HTTP server started");
}
