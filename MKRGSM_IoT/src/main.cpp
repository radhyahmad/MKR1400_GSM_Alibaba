#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "MKRGSM.h"
#include "ArduinoMqttClient.h"
#include "ArduinoJson.h"
#include "Adafruit_SHT31.h"
#include "SparkFunCCS811.h"
#include "configuration.h"

/*.................................................................................................................................*/
/*IoT device, based on arduino MKR 1400 GSM,is used to detect some parameters of air quality:Air Temperature, Humidity, CO2, and VOC*/
/*By using SHT31 and CCS811 sensors to collect some parameters of measurement*/
/*.................................................................................................................................*/

void initialize_sensor();
void initialize_gprs();
void initialize_mqtt();
void connect_mqtt();
void on_mqtt_message(int messageSize);
void publish_message();

GPRS gprs;
GSM gsmAccess;
GSMClient gsmClient;
MqttClient mqttClient(gsmClient);

Adafruit_SHT31 sht31 = Adafruit_SHT31();
uint32_t last_data = 0;
#define CSS811_ADDR 0x5A
CCS811 ccs811(CSS811_ADDR);
#define INTERVAL (uint32_t)15000

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  initialize_sensor();
  initialize_gprs();
  initialize_mqtt();
  connect_mqtt();
}
  
void loop()
{
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alives which avoids being disconnected by the broker
  mqttClient.poll();
  if (millis() - last_data > INTERVAL)
  {
    last_data = millis();
    publish_message();
  }
}

void initialize_sensor()
{
  Wire.begin();
  if (!sht31.begin(0x44))
  {
    Serial.println("Error initialising SHT31!");
  }

  if (ccs811.begin() == false)
  {
    Serial.println("Error initialising CCS811!");
    while (1);
  }
}

void initialize_gprs()
{
  //attempt to connect to GSM and GPRS
  Serial.println("Attempting to connect to GSM and GPRS");

  //connection state 
  bool connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected)
  {
    if ((gsmAccess.begin(PIN_NUMBER) == GSM_READY) && (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  Serial.println(": You're connected to the network");
  Serial.println();
}

void initialize_mqtt()
{
  mqttClient.setId(CLIENT_ID);
  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  mqttClient.onMessage(on_mqtt_message);
}

void connect_mqtt()
{
  Serial.println("Connecting MQTT .....");

  while (!mqttClient.connected())
  {
    Serial.print("Attempting to MQTT broker: ");
    //initializeMqtt();
    if (!mqttClient.connect(SECRET_BROKER, (uint32_t)1883))
    {
      Serial.print("MQTT connection failed! Error code: ");
      Serial.println(mqttClient.connectError());
      Serial.println(" Try again in 5 seconds");
      digitalWrite(LED_BUILTIN, LOW);
      delay(5000);
    }
    else
    {
      Serial.println("Connected to MQTT broker");
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println();
    }
    mqttClient.subscribe(SUBSCRIBE_TOPIC);
  }
}

void on_mqtt_message(int messageSize) 
{
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  while (mqttClient.available() != (uint32_t)0)
  {
    Serial.print((char)mqttClient.read());
  }

  Serial.println();
}

void publish_message()
{
  if (ccs811.dataAvailable())
   {
     ccs811.readAlgorithmResults();
   }
  static char payload[256];
  StaticJsonDocument<256>doc;

  doc["id"] = "a1jc2WIhUrQ.mkriot";
  JsonObject params = doc.createNestedObject("params");
  params["temperature"] = (float_t)sht31.readTemperature();
  params["humidity"] = (float_t)sht31.readHumidity();
  params["co2"] = ccs811.getCO2();
  params["voc"] = ccs811.getTVOC();
  
  doc["method"] = "thing.event.property.post";
  serializeJson(doc, payload);

  Serial.println("Publishing message: ");
  mqttClient.beginMessage(PUBLISH_TOPIC);
  mqttClient.print(payload);
  Serial.println(payload);
  mqttClient.endMessage();
}