#include <Arduino.h>

#define PIN_NUMBER ""
#define GPRS_APN "internet"
#define GPRS_LOGIN ""
#define GPRS_PASSWORD ""

// Fill in the hostname of your IoT broker
#define SECRET_BROKER "a1jc2WIhUrQ.iot-as-mqtt.cn-shanghai.aliyuncs.com" 
 
#define CLIENT_ID "a1jc2WIhUrQ.mkriot|securemode=2,signmethod=hmacsha256,timestamp=1648375030487|" 
#define MQTT_USERNAME "mkriot&a1jc2WIhUrQ" 
#define MQTT_PASSWORD "ff06ba02501af3a4ed82731dd34cfb767620962b8e7cf009dc045c9d60292ba2" 

// Fill in the publisher and subscriber topic
#define PUBLISH_TOPIC "/sys/a1jc2WIhUrQ/mkriot/thing/event/property/post"
#define SUBSCRIBE_TOPIC "/sys/a1jc2WIhUrQ/mkriot/thing/service/property/set"