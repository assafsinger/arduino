#include <LivoloTx.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

static const char* WIFI_SSID = "finchouse_ext";
static const char* WIFI_PASSWORD = "6352938635293";

static const char* PASSCODE = "ori";

static const uint16_t LIVOLO_REMOTE_ID = 6400;

static const int TX_PIN = D1;

ESP8266WebServer server(8080);

int nextCmd = 0;
int nextBtn = 0;

LivoloTx gLivolo(TX_PIN);

void onLights() {
  const String& strButton = server.arg("command");
  const String& strPassword = server.arg("secret");
  if (strPassword.equals(PASSCODE))
  {
    nextBtn = strButton.toInt();
    nextCmd = 1;
    String str = "lights on "+strButton;
    server.send(200, "text/plain", str);
  }
  else
  {
    server.send(403, "text/plain", "access denied");
  }
}

void onIndex() {
  server.send(200, "text/html", "<h1>This is the light switch!</h1>");
}

void connectWiFi()
{
  //Static IP address configuration
  IPAddress staticIP(192, 168, 1, 234); //ESP8266 static ip
  IPAddress gateway(192, 168, 1, 1); //IP Address of your WiFi Router (Gateway)
  IPAddress subnet(255, 255, 255, 0); //Subnet mask
  IPAddress dns(8, 8, 8, 8); //DNS
  //const char* deviceName = "livolo.ooo";
  WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

String inString = "";    // string to hold input
int readIntFromSerial(){
  // Read serial input:
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      Serial.print("Value:");
      Serial.println(inString.toInt());
      // clear the string for new input:
      inString = "";
      return inString.toInt();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
  
  server.on("/lights", HTTP_ANY, onLights);
  server.on("/", onIndex);
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
  }
  
  //server.handleClient();
  nextCmd = readIntFromSerial();
  if (nextCmd)
  {
    gLivolo.sendButton(LIVOLO_REMOTE_ID, nextBtn);
    Serial.print("lights on:");
    Serial.println(nextBtn);
    nextCmd = 0;
    nextBtn = 0;
  }
}

