
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <ArduinoJson.h>

// Set WiFi credentials
#define WIFI_SSID "ssid__XXX"
#define WIFI_PASS "pass__XXX"

static const uint8_t K_LICHT = D1;
static const uint8_t K_TUER = D5;
static const uint8_t K_KLINGEL =D6;
static const uint8_t K_NIX =D7;

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    //case ENC_TYPE_UNKNOWN:
    default:
      Serial.println("Unknown");
      break;
  }
}

void scan() {
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(false, true);
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ch:");
      Serial.print(WiFi.channel(i));
      Serial.print(" ");
      printEncryptionType(WiFi.encryptionType(i));
      delay(10);
    }
  }
  Serial.println("");
}

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

void initRelais(uint8_t nr){
   //digitalWrite(nr, HIGH);
   pinMode(nr, OUTPUT);
   digitalWrite(nr, HIGH);
}
  
void setup() {
  // Setup serial port
  Serial.begin(115200);
  Serial.println();
  scan();
  // Begin WiFi

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  int outs = LOW;

  initRelais(K_LICHT);
  initRelais(K_TUER);
  initRelais(K_KLINGEL);
  initRelais(K_NIX);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, outs);

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  WiFi.hostname("klingelrelais");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    // Loop continuously while WiFi is not connected
    int tryes = 30;
    while (WiFi.status() != WL_CONNECTED && tryes > 0) {
      delay(500);
      Serial.print(".");
      if (outs == HIGH) {
        outs = LOW;
      } else {
        outs = HIGH;
      }
      digitalWrite(LED_BUILTIN, outs);
      tryes--;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Not connectet. Try a reset");
      delay(500);
      WiFi.disconnect();
      delay(500);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(500);
      scan();
    }
  }
  outs = LOW;
  digitalWrite(LED_BUILTIN, outs);

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);     
  server.on("/relays", HTTP_POST, handleRelais);
   
  server.begin();
  Serial.println("Web server started!");

  outs = HIGH;
  digitalWrite(LED_BUILTIN, outs);
}

void loop(void){
  server.handleClient();
}
#ifndef MAX
#define MAX 2000
#endif

void handleRoot() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<form action=\"/relays\" method=\"POST\">";
  ptr +="<select id=\"relay\" name=\"relay\"><option value=\"";
  ptr += K_TUER;
  ptr +="\">Tuer oeffner</option>\n<option value=\"" ;
  ptr += K_LICHT;
  ptr +="\">Licht</option>\n<option value=\"";
  ptr += K_KLINGEL;
  ptr +="\">Klingel</option>\n<option value=\"";
  ptr += K_NIX;
  ptr += "\">Tuer oeffner</option>";
  ptr +="<input type=\"submit\" value=\"Toggle relais\"></form>";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  server.send(200, "text/html", ptr);
}


void handleRelais() {                          // If a POST request is made to URI /LED
  int relay = server.arg("relay").toInt();
  if (relay<1){
    Serial.println("No Relay");
  } else {
    Serial.println(relay);
    digitalWrite(relay,!digitalRead(relay));
  }
  // Redirect to main
  server.sendHeader("Location","/");
  server.send(303);
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
