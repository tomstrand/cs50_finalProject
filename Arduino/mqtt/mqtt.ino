/*

 This example connects to an unencrypted Wifi network.
 Then it prints the  MAC address of the Wifi module,
 the IP address obtained, and other network details.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFiNINA.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <DHT.h>

#include "arduino_secrets.h" 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

#define DHTPIN 1 //pin on board
#define DHTTYPE DHT22 //Sensor type

DHT dht(DHTPIN, DHTTYPE);

int status = WL_IDLE_STATUS;     // the Wifi radio's status

String topic = "/home/room1/sensor1";
String temperature;

//Variables DHT
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

// Initialize the client library
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

  // attempt to connect to mqtt client
  while (!client.connect("arduino", MQTT_USER, MQTT_PWD)){
    Serial.print("\nConnecting mqtt...");
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMqtt connected!");

  client.subscribe("/hello");
  // client.unsubscribe("/hello)");

}

void messageReceived(String &topic, String &payload) 
{
  Serial.println("incoming: " + topic + " - " + payload);
}

void sendMessage(String topic, float temp, float hum)
{
    StaticJsonDocument<200> doc;

    //Add values to the Json document
    doc["temperature"] = temp;
    doc["humidity"] = hum;

    String msg;

    serializeJson(doc, msg);
    
    client.publish(topic, msg, false, 1);

    return;
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  client.begin(MQTT_BROKER_IP, MQTT_PORT, net);
  client.onMessage(messageReceived);

  connect();
  
  dht.begin();

 
}


void loop() {
  // check the network connection once every 10 seconds:
  delay(10000);
  printCurrentNet();
  
  client.loop();

  if (!client.connected()) {
    connect();
  }


    delay(2000);
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    delay(10000); //Delay 2 sec.

    
  // publish a message roughly every 60 seconds
  if (millis() - lastMillis > 60000) {
    lastMillis = millis();
    sendMessage(topic, temp, hum);
   //client.publish("/home/room1/sensor1", doc, false, 1);
  }

}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}


 
