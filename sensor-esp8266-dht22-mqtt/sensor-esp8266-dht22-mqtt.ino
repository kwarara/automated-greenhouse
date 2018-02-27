//  Code for an ESP8266 with DHT22.
//  Detects temperature and humidity and sends to an mqtt hub.
//  Only sends data if the valu ehas changed.
//  Derived from an original thingsboard source.

/////////////////////////////////////////
// TO DO
//
// Remove thingsboard token
// Secure commuication with mqtt hub
/////////////////////////////////////////

#include "DHT.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define WIFI_AP "parkplace"
#define WIFI_PASSWORD "curlypotato730"

#define TOKEN "X4LJ3kgk0Wzyeg0IuV7C"

// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

char iotHub[] = "192.168.0.8";
unsigned int iotHubPort = 1883;
char tempTopic[] = "home/greenhouse/temp";
char humidTopic[] = "home/greenhouse/humidity";
int pause = 10000;

WiFiClient wifiClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;
String lasth;
String lastt;

void setup()
{
  Serial.begin(115200);

  //  Begin dht sensor
  dht.begin();
  delay(10);

  //  Initialise wifi
  InitWiFi();
  client.setServer( iotHub, iotHubPort );
  lastSend = 0;
  lasth = "";
  lastt = "";
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > pause ) { // Update and send only after a pause
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  client.loop();
}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  String temperature = String(round(t));
  String humidity = String(round(h));

  Serial.print( "Old temp: " );
  Serial.print( lastt ); Serial.print( ", " );
  Serial.print( "New temp: " );
  Serial.println( temperature );

  if ( temperature != lastt ) {
    Serial.println( "Temperature has changed.");    
    String payload = "{";
    payload += "\"temperature\":"; payload += temperature; payload += "}";
    char tempAttribute[50];
    payload.toCharArray( tempAttribute, 50 );
    client.publish( tempTopic, tempAttribute );
    lastt = temperature;
  } else {
    Serial.println( "Temperature hasn't changed.");
  }

  Serial.print( "Old humidity: " );
  Serial.print( lasth ); Serial.print( ", " );
  Serial.print( "New humidity: " );
  Serial.println( humidity );

  if ( humidity != lasth ) {
    Serial.println( "Humidity has changed.");
    String payload = "{";
    payload += "\"humidity\":"; payload += humidity; payload += "}";
    char humidAttribute[50];
    payload.toCharArray( humidAttribute, 50);
    client.publish( humidTopic, humidAttribute );
    lasth = humidity;
  } else {
    Serial.println( "Humidity hasn't changed.");
  }

}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to IOT hub");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}




