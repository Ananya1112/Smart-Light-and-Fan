#define BLYNK_PRINT Serial   
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define WIFI_SSID "Ananya"
#define WIFI_PASS "king1112"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "AnanyaKing"
#define MQTT_PASS "aio_PdPN596mS7GHObAp5lz2y49z91DO"
char auth[] = "3LUZ8opSYKClrpGwXU5tx46hckt9ss1Z"; // PASTE YOUR AUTHENTICATION CODE HERE        
#define LED D1
#define FAN D2
int LDR_Pin = A0;

#define DHTPIN D3
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

//Set up MQTT and WiFi clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/onoff");


void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASS); 
  dht.begin();

  //Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.println("OK!");

  //Subscribe to the onoff feed
  mqtt.subscribe(&onoff);

  pinMode(LED, OUTPUT);
  pinMode(FAN,OUTPUT);
}

void loop()
{
  Blynk.run();

   hum = dht.readHumidity();
   temp= dht.readTemperature();
   Serial.print("Humidity: ");
   Serial.print(hum);
   Serial.print(" %, Temp: ");
   Serial.print(temp);
   Serial.println(" Celsius");
   delay(3000);
  
  MQTT_connect();
  
  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    if (subscription == &onoff)
    {
      //Print the new value to the serial monitor
      Serial.print("onoff: ");
      Serial.println((char*) onoff.lastread);
      
      //If the new value is  "ON", turn the light on.
      //Otherwise, turn it off.
      if (!strcmp((char*) onoff.lastread, "ON"))
      {
        //Active low logic
        digitalWrite(LED, LOW);
      }
      else
      {
        digitalWrite(LED, HIGH);
      }
    }
  }

  // ping the server to keep the mqtt connection alive
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }

  if(analogRead(LDR_Pin) < 100)
  digitalWrite( LED, HIGH);
   
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
