#include <WiFi.h>
#include <ESPmDNS.h>
void setup_wifi()
{
  delay(10);
  Serial.print("Connecting to SSID: ");
  Serial.println(WIFI_SSID);
  // We start by connecting to a WiFi network

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_NAME);
  if (WiFi.status() != WL_CONNECTED)
  { // FIX FOR USING 2.3.0 CORE (only .begin if not connected)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    MDNS.begin(DEVICE_NAME);
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname is: ");
  Serial.println(WiFi.getHostname());
}