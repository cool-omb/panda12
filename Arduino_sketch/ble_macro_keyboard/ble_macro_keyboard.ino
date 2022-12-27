#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <FS.h>
#include <SPIFFS.h>

// 無線LANの設定　アクセスポイントのSSIDとパスワード
const char *ap_ssid = "Preference_for_keyboard"; // APのSSID
IPAddress ip(192, 168, 1, 100);
IPAddress subnet(255, 255, 255, 0);
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

String toStringIp(IPAddress ip)
{
  String res = "";
  for (int i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void captivePortal()
{
  // 無効リクエストはすべてESP32に向ける
  server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
  server.send(302, "text/plain", "");
  server.client().stop();
}

bool handleUrl(String path)
{
  if (path.endsWith("/"))
  {

    if (SPIFFS.exists("/index.html"))
    {
      File file = SPIFFS.open("/index.html", "r");
      server.streamFile(file, "text/html");
      file.close();
    }
    else
    {
      server.send(404, "text/plain", "ESP: 404 not found.");
    }
    return true;
  }
  else if (path.endsWith("/set"))
  {
  }
  else if (path.endsWith("style.css"))
  {
    if (SPIFFS.exists("/style.css"))
    {
      File file = SPIFFS.open("/style.css", "r");
      server.streamFile(file, "text/css");
      file.close();
    }
    else
    {
      server.send(404, "text/plain", "ESP: 404 not found.");
    }
    return true;
  }
  else if (path.endsWith("script.js"))
  {
    if (SPIFFS.exists("/script.js"))
    {
      File file = SPIFFS.open("/script.js", "r");
      server.streamFile(file, "text/javascript");
      file.close();
    }
    else
    {
      server.send(404, "text/plain", "ESP: 404 not found.");
    }
    return true;
  }
  else if (path.endsWith("keymap.json"))
  {
    if (SPIFFS.exists("/keymap.json"))
    {
      File file = SPIFFS.open("/keymap.json", "r");
      server.streamFile(file, "application/json");
      file.close();
    }
    else
    {
      server.send(404, "text/plain", "ESP: 404 not found.");
    }
    return true;
  }
  else if (path.endsWith("metakey.json"))
  {
    if (SPIFFS.exists("/metakey.json"))
    {
      File file = SPIFFS.open("/metakey.json", "r");
      server.streamFile(file, "application/json");
      file.close();
    }
    else
    {
      server.send(404, "text/plain", "ESP: 404 not found.");
    }
    return true;
  }
  return false;
}

void setup()
{
  // SPIFFSのセットアップ
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // 無線LAN接続APモード
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, ip, subnet);
  WiFi.softAP(ap_ssid);
  dnsServer.start(DNS_PORT, "*", ip);

  server.onNotFound([]()
                    {
        if (!handleUrl(server.uri())) {
            captivePortal(); //ESP32のページにリダイレクトする capative portalの仕組み
        } });
  server.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  server.handleClient();
}