/**
 * @file MqttClient.cpp
 * @brief MQTT客户端实现类 - 重构版本
 * @description 提供数据包创建、发布、接收解析等功能
 */

#include "MqttClient.h"

    // 使用宏定义的配置参数
const char* MqttClient::ssid = WIFI_SSID;
const char* MqttClient::password = WIFI_PASSWORD;
const char* MqttClient::mqtt_server = MQTT_SERVER;
const int MqttClient::mqtt_port = MQTT_PORT;
const char* MqttClient::mqtt_user = MQTT_USER;
const char* MqttClient::mqtt_password = MQTT_PASSWORD;
const char* MqttClient::mqtt_topic = MQTT_TOPIC;
const char* MqttClient::device_id = DEVICE_ID;


// 添加静态实例指针定义
MqttClient* MqttClient::instance = nullptr;

/**
 * @brief 构造函数 - 初始化MQTT客户端
 */
MqttClient::MqttClient() : client(espClient) {

    // 设置静态实例指针
    instance = this;

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

/**
 * @brief 开始初始化流程
 */
void MqttClient::begin() {
    setupWiFi();
}

/**
 * @brief 设置WiFi连接
 */
void MqttClient::setupWiFi() {
    delay(10);
    Serial.println();
    Serial.print("正在连接到WiFi网络: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("✓ WiFi连接成功！");
    Serial.print("本地IP地址: ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief 重新连接MQTT服务器
 */
void MqttClient::reconnect() {
    while (!client.connected()) {
        Serial.print("正在尝试连接MQTT服务器...");
        
        String clientId = "ESP32-master-Client-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("✓ MQTT连接成功！");
            client.subscribe(mqtt_topic);
            Serial.print("已订阅主题: ");
            Serial.println(mqtt_topic);
        } else {
            Serial.print("✗ 连接失败，错误代码: ");
            Serial.print(client.state());
            Serial.println(" | 3秒后重新尝试连接...");
            delay(3000);
        }
    }
}

/**
 * @brief 主循环处理函数
 */
void MqttClient::loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

// ==================== 发布函数 ====================

/**
 * @brief 发送数据 - Map版本（用于复杂数据结构）
 * @param dataType 数据类型
 * @param data 数据键值对
 * @return 发送是否成功
 */
bool MqttClient::sendData(const String& dataType, const std::map<String, String>& data) {
    JsonDocument doc;
    
    // 固定字段
    doc["device_id"] = device_id;  // 注意这里应该是deviceId而不是device_id
    doc["timestamp"] = millis();
    doc["data_type"] = dataType;
    
    // 动态添加数据字段
    for (const auto& pair : data) {
        doc[pair.first] = pair.second;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);

    // Serial.println(jsonString);
    
    if (client.connected()) {
        bool result = client.publish(mqtt_topic, jsonString.c_str());
        if (result) {
            Serial.print("✅ 发送数据成功: ");
            Serial.println(jsonString);
        } else {
            Serial.println("❌ 发送数据失败");
        }
        return result;
    } else {
        Serial.println("❌ MQTT未连接");
        return false;
    }
}
// 便捷函数实现
bool MqttClient::sendTemperatureHumidity(float temp, float humi) {
    std::map<String, String> sensorData = {
        {"temp", String(temp)},
        {"humi", String(humi)}
    };
    return sendData("temp_humi", sensorData);
}

// ==================== 接收和解析函数 ====================

/**
 * @brief MQTT消息接收回调函数
 * @param topic 消息主题
 * @param payload 消息内容（字节数组）
 * @param length 消息长度
 */
void MqttClient::callback(char* topic, byte* payload, unsigned int length) {
    // 将字节数组转换为字符串
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.println("\n📩 收到MQTT消息:");
    Serial.print("  主题: ");
    Serial.println(topic);
    Serial.print("  原始内容: ");
    Serial.println(message);
    
    // 尝试解析JSON数据
    // 通过静态实例指针调用非静态成员函数
    if (instance != nullptr) {
        instance->parseReceivedData(message);
    } else {
        Serial.println("✗ 错误：MqttClient实例指针为空！");
    }
}


/**
 * @brief 解析接收到的JSON数据（支持键值对格式）
 * @param jsonString 接收到的JSON字符串
 */
void MqttClient::parseReceivedData(const String& jsonString) {
    // 创建JSON文档用于解析
    JsonDocument doc;
    
    // 反序列化JSON字符串
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.print("✗ JSON解析失败: ");
        Serial.println(error.c_str());
        Serial.println("  尝试作为普通文本处理...");
        
        // 如果不是JSON格式，作为普通文本处理
        Serial.println("📝 解析结果（纯文本）:");
        Serial.print("  内容: ");
        Serial.println(jsonString);
        return;
    }
    
    // JSON解析成功，提取各个字段
    Serial.println("✓ JSON解析成功:");
    
    // 提取基本信息
    String deviceId = doc["device_id"] | "未知设备";
    unsigned long timestamp = doc["timestamp"] | 0;
    String dataType = doc["data_type"] | "未知类型";
    
    Serial.print("  设备ID: ");
    Serial.println(deviceId);
    Serial.print("  时间戳: ");
    Serial.println(timestamp);
    Serial.print("  数据类型: ");
    Serial.println(dataType);
    
    // 提取所有数据字段（除了固定字段）
    Serial.println("  数据内容:");
    JsonObject rootObj = doc.as<JsonObject>();
    for (JsonPair kv : rootObj) {
        String key = kv.key().c_str();
        
        // 跳过固定字段，只显示数据字段
        if (key != "device_id" && key != "timestamp" && key != "data_type") {
            Serial.print("    " + key + ": ");
            
            if (kv.value().is<float>()) {
                Serial.println(kv.value().as<float>());
            } else if (kv.value().is<int>()) {
                Serial.println(kv.value().as<int>());
            } else if (kv.value().is<bool>()) {
                Serial.println(kv.value().as<bool>() ? "true" : "false");
            } else {
                Serial.println(kv.value().as<String>());
            }
        }
    }
    
   // 🔄 查找并调用相应的数据处理函数
    auto it = dataHandlers.find(dataType);
    if (it != dataHandlers.end()) {
        Serial.print("🔄 调用数据处理函数: ");
        Serial.println(dataType);
        it->second(rootObj);  // 传递完整的JSON对象给处理函数
    } else {
        Serial.print("⚠️  未找到数据类型的处理函数: ");
        Serial.println(dataType);
        
        // 默认处理
        handleUnknownDataType(rootObj, dataType, deviceId);
    }
    
    Serial.println("📥 消息解析完成\n");
}
/**
 * @brief 处理未知数据类型
 * @param data JSON数据对象
 * @param dataType 数据类型
 * @param fromDeviceId 来源设备ID
 */
void MqttClient::handleUnknownDataType(const JsonObject& data, const String& dataType, const String& fromDeviceId) {
    Serial.println("🔍 处理未知数据类型:");
    Serial.print("  数据类型: ");
    Serial.println(dataType);
    Serial.print("  来源设备: ");
    Serial.println(fromDeviceId);
    
    // 提取并显示所有数据字段
    Serial.println("  数据详情:");
    for (JsonPair kv : data) {
        String key = kv.key().c_str();
        
        // 跳过固定字段
        if (key != "device_id" && key != "timestamp" && key != "data_type") {
            Serial.print("    " + key + ": ");
            
            if (kv.value().is<float>()) {
                Serial.println(kv.value().as<float>());
            } else if (kv.value().is<int>()) {
                Serial.println(kv.value().as<int>());
            } else if (kv.value().is<bool>()) {
                Serial.println(kv.value().as<bool>() ? "true" : "false");
            } else {
                Serial.println(kv.value().as<String>());
            }
        }
    }
}

/**
 * @brief 注册数据类型的处理函数
 * @param dataType 数据类型
 * @param handler 处理函数
 */
void MqttClient::registerDataHandler(const String& dataType, DataHandlerFunction handler) {
    dataHandlers[dataType] = handler;
    Serial.print("📝 注册数据处理函数: ");
    Serial.println(dataType);
}

/**
 * @brief 注册所有数据处理函数
 * @param mqtt MQTT客户端实例的引用
 */
void MqttClient::registerDataHandlers() {
    Serial.println("📝 注册数据处理函数...");
    
    // 1. 温湿度数据处理函数 - 使用lambda包装器
    registerDataHandler("temp_humi", [this](const JsonObject& data) {
        handleTempHumiData(data);
    });
    
    
    
    Serial.println("✅ 所有数据处理函数注册完成!");
}

/**
 * @brief 处理温湿度数据
 * @param data JSON数据对象
 * @param mqtt MQTT客户端实例的引用
 */
void MqttClient::handleTempHumiData(const JsonObject& data) {
    String deviceId = data["device_id"];
    unsigned long timestamp = data["timestamp"];
    float temp = data["temp"];
    float humi = data["humi"];
    
    Serial.println("🌡️💧 处理温湿度数据");
    Serial.print("  来源设备: ");
    Serial.println(deviceId);
    Serial.print("  温度: ");
    Serial.print(temp);
    Serial.println("°C");
    Serial.print("  湿度: ");
    Serial.print(humi);
    Serial.println("%");
    
}