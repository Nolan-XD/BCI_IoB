#include <WiFi.h>
#include <PubSubClient.h>

#define LED_PIN 2


// WiFi信息
const char* ssid = "nolanphone";
const char* password = "nolan123";

// MQTT服务器
const char* mqtt_server = "172.16.0.2";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = LED_PIN;  // 板载LED，一般GPIO2

unsigned long previousMillis = 0;
unsigned long interval = 500;  // 默认闪烁间隔500ms
bool ledState = LOW;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  for (unsigned int i=0; i<length; i++) {
    Serial.write(payload[i]);
  }
  Serial.println();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("esp32Client")) {
      Serial.println("connected");
      client.subscribe("inTopic");
      client.publish("outTopic", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // 根据连接状态设置闪烁间隔
  if (WiFi.status() != WL_CONNECTED) {
    interval = 200;  // WiFi未连接快闪
  } else if (!client.connected()) {
    interval = 200;  // MQTT未连接快闪
  } else {
    interval = 1000; // 全连接成功慢闪
    client.loop();
  }

  // 处理LED闪烁
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
  }

  // 如果MQTT未连接，尝试重连
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnect();
  }
}
