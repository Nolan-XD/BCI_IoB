#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// WiFi 配置
const char* ssid = "nolanphone";
const char* password = "nolan123";

// MQTT 配置
const char* mqtt_server = "192.168.163.155";  // 例如：192.168.1.100
const int mqtt_port = 1883;
const char* mqtt_user = "admin";
const char* mqtt_password = "pdd123456";
const char* mqtt_topic = "bci-iob-imu";

// LED 引脚
#define LED_PIN 2

// DHT11 配置
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 设备ID
const char* device_id = "ESP32_master";

WiFiClient espClient;
PubSubClient client(espClient);

// 连接WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("连接到 ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi已连接");
  Serial.println("IP地址: ");
  Serial.println(WiFi.localIP());
}

// MQTT回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("收到消息 [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// 重新连接MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("尝试MQTT连接...");
    String clientId = "ESP32-master-Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("已连接");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("失败, rc=");
      Serial.print(client.state());
      Serial.println(" 5秒后重试");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // dht.begin();
    // 读取传感器数据
  float temp = 25;
  float humi = 10;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 读取传感器数据
  temp ++;
  humi ++;

  // // 检查是否读取成功
  // if (isnan(temp) || isnan(humi)) {
  //   Serial.println("从DHT11传感器读取失败!");
  //   delay(2000);
  //   return;
  // }

  // 创建JSON数据
  StaticJsonDocument<200> doc;
  doc["temp"] = temp;
  doc["humi"] = humi;
  doc["device_id"] = device_id;

  String jsonString;
  serializeJson(doc, jsonString);

  // 发送数据
  if (client.publish(mqtt_topic, jsonString.c_str())) {
    Serial.println("数据发送成功");
  } else {
    Serial.println("数据发送失败");
  }

  // 等待一段时间再次读取
  delay(5000);
} 