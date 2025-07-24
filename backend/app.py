# backend/app.py

from flask import Flask, request
import paho.mqtt.client as mqtt
import requests
import time
import json
from config import Config
from flask_cors import CORS
from flask_socketio import SocketIO
import os
import subprocess

import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# --------------------- 初始化 Flask 和 Socket.IO ---------------------

auth = ('root', 'taosdata')
url = "http://tdengine:6041/rest/sql"
payload = "SHOW DATABASES"
response = requests.post(url, data=payload, auth=auth)
print(response.json())

app = Flask(__name__)
app.config.from_object(Config)
CORS(app, resources={r"/*": {"origins": "*"}})
socketio = SocketIO(
    app,
    cors_allowed_origins="*",
    async_mode='eventlet',
    transports=['websocket'],
    logger=True,
    engineio_logger=True,
    ping_timeout=60,
    ping_interval=30
)

# --------------------- 路由和事件 ---------------------
@app.route('/test', methods=['GET'])
def test_push():
    temp, humi = 25.0, 60.0
    socketio.emit('update', {'temp': temp, 'humi': humi})
    return "Test data sent!"

@socketio.on('connect')
def handle_connect():
    print("✅ 客户端已连接:", request.sid)
    logger.info("✅ 客户端已连接:", request.sid)

@socketio.on('disconnect')
def handle_disconnect():
    print("❌ 客户端已断开:", request.sid)
    logger.info("❌ 客户端已断开:", request.sid)

# --------------------- MQTT 回调函数 ---------------------
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("✅ MQTT Connected!")
        result, mid = client.subscribe("bci-iob-imu")
        print(f"🔗 订阅状态: {result}, 消息 ID: {mid}")
        logger.info(f"🔗 订阅状态: {result}, 消息 ID: {mid}")
    else:
        print(f"❌ MQTT 连接失败，错误代码: {reason_code}")
        logger.info(f"❌ MQTT 连接失败，错误代码: {reason_code}")

def on_sensor_data(client, userdata, msg):
    try:
        print(f"📩 收到 MQTT 消息: topic={msg.topic}, payload={msg.payload}")
        logger.info(f"📩 收到 MQTT 消息: topic={msg.topic}, payload={msg.payload}")
        data = json.loads(msg.payload)
        temp = data.get("temp")
        humi = data.get("humi")
        
        sql = f"INSERT INTO demo.sensors USING demo.sensors TAGS('device1') VALUES (NOW, {temp}, {humi})"
        logger.info("执行 SQL: %s", sql)

        response = requests.post(
            app.config["TDENGINE_URL"],
            auth=app.config["TDENGINE_AUTH"],
            data=sql
        )
        logger.info(f"TDengine 响应状态码: {response.status_code}")
        logger.info(f"TDengine 响应内容: {response.text}")
        response.raise_for_status()
        print(f"📡 正在推送数据到前端: temp={temp}, humi={humi}")
        logger.info(f"📡 正在推送数据到前端: temp={temp}, humi={humi}")
        socketio.emit("update", {"temp": temp, "humi": humi})
    except Exception as e:
        print(f"❌ 处理MQTT消息失败: {e}")
        logger.info(f"❌ 处理MQTT消息失败: {e}")

# --------------------- MQTT 初始化 ---------------------
mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_sensor_data
mqtt_client.username_pw_set("admin", "pdd123456")

# --------------------- 数据库初始化 ---------------------
def init_database():
    print("⏳ 开始初始化数据库...")
    max_retries = 10
    for i in range(max_retries):
        try:
            requests.post(
                app.config["TDENGINE_URL"],
                auth=app.config["TDENGINE_AUTH"],
                data="CREATE DATABASE IF NOT EXISTS demo"
            )
            requests.post(
                app.config["TDENGINE_URL"],
                auth=app.config["TDENGINE_AUTH"],
                data="CREATE TABLE IF NOT EXISTS demo.sensors (ts TIMESTAMP, temp FLOAT, humi FLOAT)"
            )
            print("✅ 数据库初始化成功")
            logger.info("✅ 数据库初始化成功")
            return
        except requests.exceptions.ConnectionError:
            print(f"⏳ 等待 TDengine 启动... ({i+1}/{max_retries})")
            logger.info(f"⏳ 等待 TDengine 启动... ({i+1}/{max_retries})")
            time.sleep(5)
    raise RuntimeError("无法连接 TDengine")

@app.route('/test_tdengine', methods=['GET'])
def test_tdengine_connection():
    try:
        response = requests.post(
            Config.TDENGINE_URL,
            data="SHOW DATABASES",
            auth=Config.TDENGINE_AUTH
        )
        return response.json()
    except Exception as e:
        return {"error": str(e)}, 500

def verify_tdengine_status():
    try:
        print("正在验证 TDengine 数据库状态...")
        logger.info("正在验证 TDengine 数据库状态...")
         # 使用 REST API 验证 TDengine 状态
         # 这里假设 TDengine REST API 已经正确配置并可用
         # 如果你使用的是其他方式连接 TDengine，请根据实际情况修改
        response = requests.post(
            app.config["TDENGINE_URL"],  # 通常是 http://tdengine:6041/rest/sql
            auth=app.config["TDENGINE_AUTH"],
            data="SHOW DATABASES"
        )
        response.raise_for_status()
        result = response.json()
        print("✅ TDengine 数据库状态正常:")
        logger.info("✅ TDengine 数据库状态正常:")
        for row in result.get("data", []):
            print("📦 数据库:", row[0])
            logger.info(f"📦 数据库: {row[0]}")
    except Exception as e:
        print(f"❌ TDengine 验证失败: {e}")
        logger.info(f"❌ TDengine 验证失败: {e}")


# def verify_tdengine_status():
#     try:
#         print("正在验证 TDengine 数据库状态...")
#         # output = subprocess.check_output("taos -s 'SHOW DATABASES;'", shell=True)
#         # print(output.decode())
#     except subprocess.CalledProcessError as e:
#         print(f"❌ TDengine 验证失败: {e}")

# --------------------- 启动任务封装 ---------------------
def startup_tasks():
    with app.app_context():
        init_database()
        # 启动 MQTT 客户端
        max_retries = 10
        # 设置用户名和密码
        mqtt_client.username_pw_set(
            app.config.get("MQTT_USERNAME", ""),
            app.config.get("MQTT_PASSWORD", "")
        )
        print("✅ MQTT 密码设置成功")
        logger.info("✅ MQTT 密码设置成功")
        for i in range(max_retries):
            try:
                host = app.config["MQTT_BROKER"]
                port = int(app.config["MQTT_PORT"])
                print(f"连接的 host: {host}，类型: {type(host)}")
                logger.info(f"连接的 host: {host}，类型: {type(host)}")
                print(f"连接的 port: {port}，类型: {type(port)}")
                logger.info(f"连接的 port: {port}，类型: {type(port)}")
                mqtt_client.connect(
                    host=host,
                    port=port,
                    keepalive=60
                )
                mqtt_client.loop_start()
                print("✅ MQTT 连接成功")
                break
            except Exception as e:
                print(f"❌ 连接 MQTT 失败 ({i+1}/{max_retries}): {e}")
                logger.info(f"❌ 连接 MQTT 失败 ({i+1}/{max_retries}): {e}")
                time.sleep(5)
        else:
            raise RuntimeError("无法连接 MQTT 服务器")
        
        verify_tdengine_status()

# ✅ 启动时自动运行初始化任务（适配 gunicorn）
startup_tasks()

# 🟡 仅用 python app.py 时运行 socketio.run()
if __name__ == "__main__":
    socketio.run(app, host="0.0.0.0", port=5000, debug=True, use_reloader=False)
