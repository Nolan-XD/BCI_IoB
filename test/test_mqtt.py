'''
Author: Nolan-XD haopengliuu@163.com
Date: 2025-07-23 14:49:18
LastEditors: Nolan-XD haopengliuu@163.com
LastEditTime: 2025-07-23 16:51:55
FilePath: \BCI_IoB\test\BCI-IoB-test\test_mqtt.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import paho.mqtt.client as mqtt
import json
import time

# MQTT 配置
MQTT_BROKER = "localhost"  # 或者容器名 emqx，如果你在 Docker 内部运行
MQTT_PORT = 1883
MQTT_TOPIC = "sensors/dht11"
MQTT_USERNAME = "admin"
MQTT_PASSWORD = "pdd123456"

# 构造测试数据
data = {
    "temp": 400,
    "humi": 10000
}

# 创建 MQTT 客户端
client = mqtt.Client()

# 设置用户名和密码
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

# 连接 broker
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# 发布消息
payload = json.dumps(data)
result = client.publish(MQTT_TOPIC, payload)

# 检查是否成功
if result.rc == mqtt.MQTT_ERR_SUCCESS:
    print(f"✅ 已向 {MQTT_TOPIC} 发送数据: {payload}")
else:
    print("❌ 发送失败")

# 可选：停顿一下再断开
time.sleep(1)
client.disconnect()
