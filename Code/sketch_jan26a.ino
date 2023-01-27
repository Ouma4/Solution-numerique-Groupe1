#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define wifi_ssid "Invite-ESIEA"
#define wifi_password "hQV86deaazEZQPu9a" 

#define mqtt_server "10.8.128.250"



#define topic "ESIEA/grp1"  

char message_buff[100];

long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;
bool debug = true; 

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topicRecu, byte* payload, unsigned int length) {

  int i = 0;
  Serial.println("Message recu =>  topic: " + String(topicRecu));
  Serial.print(" | longueur: " + String(length,DEC));
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();           
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);   
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)");
  dht.setup(4, DHTesp::DHT22); // Connect DHT sensor to D2 GPIO4
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connexion WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}

void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect("Admin")) {
      Serial.println("OK");
    } else {
      Serial.print("KO, erreur : ");
      Serial.print(client.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(5000);
    }
  }
}

void loop()
{

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(2000);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.println(temperature, 1);

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<500> jsonBuffer;

  // Create the root object
  JsonObject& JsonDataToBroker = jsonBuffer.createObject();
  String DataToBroker;
  
  JsonDataToBroker["Temperature"] = temperature; //Put Sensor value
  JsonDataToBroker["Humidity"] = humidity; //Reads Flash Button Status
  JsonDataToBroker.printTo(DataToBroker);
  Serial.print(DataToBroker);
  Serial.print("\n");
  client.publish(topic,String(DataToBroker).c_str(), true);   //Publie la température sur le topic temperature_topic
  client.subscribe(topic);
}