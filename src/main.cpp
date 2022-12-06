#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "time.h"

//Constants
#define dht22 2
#define humidifier 4
#define lightcontrol 5
#define heater 18
#define cooler 19
#define vcc 21
#define DHTTYPE DHT22


const char* ntpServer = "np.pool.ntp.org";
const long  gmtOffset_sec = 20700;
const int   daylightOffset_sec = 0;

float hum;  //Stores humidity value
float temp; //Stores temperature value

struct tm timeinfo;

// bool isWiFi = false;
// bool wasConnectionLossed = true;
 
#define AWS_IOT_PUBLISH_TOPIC "esp32/dht22"
#define AWS_IOT_SUBSCRIBE_TOPIC1 "esp32/humidifier"
#define AWS_IOT_SUBSCRIBE_TOPIC2 "esp32/lightcontrol"
#define AWS_IOT_SUBSCRIBE_TOPIC3 "esp32/heater"
#define AWS_IOT_SUBSCRIBE_TOPIC4 "esp32/cooler"
 

DHT dht(dht22, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

 
void publishMessage()
{
time_t now = time(NULL);
char *ct = ctime(&now);
  StaticJsonDocument<200> doc;
  doc["humidity"] = hum;
  doc["temperature"] = temp;
 doc["timestamp"] = ct;
//  doc["wasConnectionLossed"]= wasConnectionLossed;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);


  Serial.print(ct);
  Serial.print(F("Humidity: "));
  Serial.print(hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
  Serial.println(F("°C "));

}


void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
/*##################### humidifier #####################*/
  if ( strstr(topic, "esp32/humidifier") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay1 = doc["status"];
    int r1 = Relay1.toInt();
    if(r1==1)
    {
      digitalWrite(humidifier, LOW);
      Serial.print("Humidifier is ON");
    }
    else if(r1==0)
    {
      digitalWrite(humidifier, HIGH);
      Serial.print("Humidifier is OFF");
    }
  }
 
/*##################### Light Control #####################*/
  if ( strstr(topic, "esp32/lightcontrol") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay2 = doc["status"];
    Serial.print(Relay2);
    int r2 = Relay2.toInt();
    if(r2==1)
    {
      digitalWrite(lightcontrol, LOW);
      Serial.print("lightcontrol is ON");
    }
    else if(r2==0)
    {
      digitalWrite(lightcontrol, HIGH);
      Serial.print("lightcontrol is OFF");
    }
  }
 
/*##################### Heater #####################*/
  if ( strstr(topic, "esp32/heater") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay3 = doc["status"];
    int r3 = Relay3.toInt();
    if(r3==1)
    {
      digitalWrite(heater, LOW);
      Serial.print("heater is ON");
    }
    else if(r3==0)
    {
      digitalWrite(heater, HIGH);
      Serial.print("heater is OFF");
    }
  }
 
/*##################### Cooler #####################*/
  if ( strstr(topic, "esp32/cooler") )
  {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    String Relay4 = doc["status"];
    int r4 = Relay4.toInt();
    if(r4==1)
    {
      digitalWrite(cooler, LOW);
      Serial.print("cooler is ON");
    }
    else if(r4==0)
    {
      digitalWrite(cooler, HIGH);
      Serial.print("cooler is OFF");
    }
  }
  Serial.println();
}
 
 
void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode (dht22, INPUT);
  pinMode (humidifier, OUTPUT);
  pinMode (lightcontrol, OUTPUT);
  pinMode (heater, OUTPUT);
  pinMode (cooler, OUTPUT);
  pinMode (vcc, OUTPUT);
 
  digitalWrite(humidifier, HIGH);
  digitalWrite(lightcontrol, HIGH);
  digitalWrite(heater, HIGH);
  digitalWrite(cooler, HIGH);
  digitalWrite(vcc, HIGH);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
//   wasConnectionLossed = false;
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC1);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC3);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC4);
 
  Serial.println("AWS IoT Connected!");
}
 
 
void loop()
{
    // if (WiFi.status() == WL_CONNECTED && !wasConnectionLossed ){
    //     wasConnectionLossed = false;
    //     delay(500);
    // }
    // else if(WiFi.status() != WL_CONNECTED){
    //     Serial.println("WARNING! You are offline");
    //     wasConnectionLossed = true;
    //     delay(500);
    // }

  hum = dht.readHumidity();
  temp = dht.readTemperature();
 
 
  if (isnan(hum) || isnan(temp) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(2000);
    return;    
  } 
 
  publishMessage();
  client.loop();
  delay(1000);
}