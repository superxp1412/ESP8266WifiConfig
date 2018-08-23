#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WifiConfig.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <WString.h>

WifiConfig wifiConfig;

void setup()
{
	// init serial port
	Serial.begin(9600);

	// uncomment the following if you set a static IP in the begining
	// WiFi.config(nkip, nkgateway, nksubnet);

	// call espWiFi2eeprom to connect to saved to eeprom AP or to create an AP to store new values for SSID and password
	wifiConfig.espNKWiFiconnect();

	// show IP address
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	// init basic web server for basic functions: info, restart, cleareeprom
	wifiConfig.initBasicHttpServer();
}

void loop()
{
	wifiConfig.server.handleClient();
}
