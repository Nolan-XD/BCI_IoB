#include <Arduino.h>
#include "Timer.h"
#include "MqttClient.h"

// 创建MQTT客户端实例
MqttClient mqttClient;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // 5秒发送一次数据
int dataCounter = 0;


// LED 引脚
#define LED_PIN 2

  // 读取传感器数据
float temp = 25;
float humi = 10;

// 使用宏定义
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;
const char* device_id = DEVICE_ID;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 启动定时器系统...");
    
    // 初始化定时器
    if (timer.init()) {
        Serial.println("✅ 定时器系统就绪");
    } else {
        Serial.println("❌ 定时器初始化失败");
        return;
    }

    // 初始化定时器
    if (timer.ledInit()) {
        Serial.println("✅ LED初始化成功");
    } else {
        Serial.println("❌ LED初始化失败");
        return;
    }
  
  // 初始化MQTT客户端
  Serial.println("\n🚀 ESP32 MQTT客户端启动");
  Serial.println("使用platformio.ini宏定义配置");
  // 打印配置信息（调试用）
  Serial.println("📡 网络配置:");
  Serial.print("  WiFi SSID: ");
  Serial.println(ssid);
  Serial.print("  MQTT Server: ");
  Serial.println(mqtt_server);
  Serial.print("  MQTT Port: ");
  Serial.println(mqtt_port);
  Serial.print("  Device ID: ");
  Serial.println(device_id);
  mqttClient.begin();

  // 注册所有数据处理函数
  mqttClient.registerDataHandlers();
  

  Serial.println("✅ MQTT初始化完成\n");

  

}

void loop() {
   // 不同频率的LED闪烁示例
    timer.ledTick(Timer::TimeFlag::FLAG_2S);  // 500ms闪烁
    
    
    if (timer.getTimeFlag(Timer::TimeFlag::FLAG_10MS)) {
        // 执行10ms周期任务
        // ...
        timer.clearTimeFlag(Timer::TimeFlag::FLAG_10MS);
    }
  

  // 维护MQTT连接
  mqttClient.loop();


  // 定时发送测试数据
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
      lastSendTime = currentTime;
      dataCounter++;
      
      // 发送不同类型的测试数据
      switch (dataCounter % 4) {
          case 0:
              // 发送温度数据
              mqttClient.sendTemperatureHumidity(temp+=10, humi+=10);
              break;
              
          case 1:
              
              break;
              
          case 2:
              
              break;
              
          case 3:
              
              break;
      }
  }
} 