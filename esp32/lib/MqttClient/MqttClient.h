/**
 * @file MqttClient.h
 * @brief MQTT客户端头文件 - 使用宏定义配置
 */

#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <map>

// 从platformio.ini获取配置参数的宏定义检查
#ifndef WIFI_SSID
#define WIFI_SSID "default_ssid"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "default_password"
#endif
#ifndef MQTT_SERVER
#define MQTT_SERVER "192.168.1.1"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER "user"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "password"
#endif
#ifndef MQTT_TOPIC
#define MQTT_TOPIC "topic"
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "device"
#endif

class MqttClient {
private:
    WiFiClient espClient;
    PubSubClient client;
    
    // 静态配置参数
    static const char* ssid;
    static const char* password;
    static const char* mqtt_server;
    static const int mqtt_port;
    static const char* mqtt_user;
    static const char* mqtt_password;
    static const char* mqtt_topic;
    static const char* device_id;

    PubSubClient mqttClient;

    // 添加静态实例指针，用于在静态回调函数中访问实例方法
    static MqttClient* instance;

    // 数据处理函数类型定义
    using DataHandlerFunction = std::function<void(const JsonObject&)>;
    
    // 存储数据处理函数的map
    std::map<String, DataHandlerFunction> dataHandlers;
    
public:
    MqttClient();
    
    // 初始化和连接
    void begin();
    void setupWiFi();
    void reconnect();
    void loop();

    // ==================== 发布函数 ====================

    // 通用发送函数 - 支持任意数据类型和字段
    bool sendData(const String& dataType, const std::map<String, String>& data);

    // 便捷的特定类型发送函数
    bool sendTemperatureHumidity(float temp, float humi);
    
    // ==================== 接收和解析函数 ====================
    
    static void callback(char* topic, byte* payload, unsigned int length);
    void parseReceivedData(const String& jsonString);
    void handleParsedData(const String& dataType, const String& content, const String& deviceId);
    // 注册数据处理函数
    void handleUnknownDataType(const JsonObject& data, const String& dataType, const String& fromDeviceId);
    void registerDataHandler(const String& dataType, DataHandlerFunction handler);
    void registerDataHandlers();
    // 处理温湿度数据
    void handleTempHumiData(const JsonObject& data) ;


};