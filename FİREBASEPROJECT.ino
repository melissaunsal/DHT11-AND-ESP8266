

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
#include <ESP8266WebServer.h>
#endif
#include "DHT.h"
#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define LED_2 D2
#define LED D3
#define DHTPIN D4
#define BUZZER D1
#define DHTTYPE DHT11
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
WiFiServer server(80);
#define API_KEY ""
#define DATABASE_URL ""
#define USER_EMAIL ""
#define USER_PASSWORD ""
FirebaseData fbdo;
#define BOTtoken ""  
#define CHAT_ID ""

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure clientb;
UniversalTelegramBot bot(BOTtoken, clientb);

FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
DHT dht(DHTPIN, DHTTYPE);


void setup() {


  dht.begin();
  Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org");      
  clientb.setTrustAnchors(&cert); 
  delay(10);
  Serial.println();
  pinMode(D1, OUTPUT);
  WiFi.mode(WIFI_STA);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(WIFI_SSID);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  Serial.println();
 bot.sendMessage(CHAT_ID, "Bot Başlatıldı", "");

  server.begin();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);
}

void loop() {
  pinMode(D3, OUTPUT);
  pinMode(D2, OUTPUT);
  WiFiClient client = server.available();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity % : ");
  Serial.println(h);
  Serial.print("Temperature *C: ");
  Serial.println(t);
  Serial.print("Temperature *F: ");
  Serial.println(f);

  if (t <= 10) {



    digitalWrite(D2, HIGH);
    digitalWrite(D3, LOW);
    digitalWrite(D1, LOW);
  }



  else {


    digitalWrite(D3, HIGH);
    digitalWrite(D2, LOW);
    digitalWrite(D1, HIGH);
    bot.sendMessage(CHAT_ID, "YÜKSEK SICAKLIK ALARMI!!", "");
  }


  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
  client.println("<head>\n<meta charset='UTF-8'>");
  client.println("<title>ESP8266 Temperature & Humidity DHT11 Sensor</title>");
  client.println("</head>\n<body>");
  client.println("<H2>ESP8266 & DHT11 Sensor</H2>");
  client.println("<H3>Humidity / Temperature</H3>");
  client.println("<pre>");
  client.print("Humidity (%)         : ");
  client.println((float)h, 2);
  client.print("Temperature (°C)  : ");
  client.println((float)t, 2);
  client.print("Temperature (°F)  : ");
  client.println((float)f, 2);
  client.println("</pre>");
  client.println("<H3>GÖMÜLÜ SİSTEMLER</H3>");
  client.print("</body>\n</html>");
  delay(2000);


  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    Serial.printf("Set Temperature... %s\n", Firebase.setFloat(fbdo, F("/test/temperature"), t) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get Temperature... %s\n", Firebase.getFloat(fbdo, F("/test/temperature")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set Humidity... %s\n", Firebase.setDouble(fbdo, F("/test/humidity"), h) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get Humidity... %s\n", Firebase.getDouble(fbdo, F("/test/humidity")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());

    FirebaseJson json;

    if (count == 0) {
      json.set("value/round/" + String(count), F("cool!"));
      json.set(F("vaue/ts/.sv"), F("timestamp"));
      Serial.printf("Set json... %s\n", Firebase.set(fbdo, F("/test/json"), json) ? "ok" : fbdo.errorReason().c_str());
    } else {
      json.add(String(count), "smart!");
      Serial.printf("Update node... %s\n", Firebase.updateNode(fbdo, F("/test/json/value/round"), json) ? "ok" : fbdo.errorReason().c_str());
    }

    Serial.println();



    count++;
  }
}
