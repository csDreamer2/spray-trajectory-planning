#!/usr/bin/env python3
"""
Unity通信测试客户端
模拟Unity应用程序与Qt应用的TCP通信
"""

import socket
import json
import time
import threading

class UnityTestClient:
    def __init__(self, host='localhost', port=12346):
        self.host = host
        self.port = port
        self.socket = None
        self.running = False
        
    def connect(self):
        """连接到Qt应用程序"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.running = True
            print(f"已连接到Qt应用程序 {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"连接失败: {e}")
            return False
    
    def disconnect(self):
        """断开连接"""
        self.running = False
        if self.socket:
            self.socket.close()
            print("已断开连接")
    
    def send_message(self, message_type, data=None):
        """发送消息到Qt应用程序"""
        if not self.socket:
            return False
            
        message = {
            "type": message_type,
            "timestamp": time.time()
        }
        
        if data:
            message["data"] = data
            
        try:
            json_str = json.dumps(message) + "\n"
            self.socket.send(json_str.encode('utf-8'))
            print(f"发送消息: {message_type}")
            return True
        except Exception as e:
            print(f"发送消息失败: {e}")
            return False
    
    def listen_for_messages(self):
        """监听来自Qt的消息"""
        buffer = ""
        while self.running:
            try:
                data = self.socket.recv(1024).decode('utf-8')
                if not data:
                    break
                    
                buffer += data
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    if line.strip():
                        try:
                            message = json.loads(line)
                            self.handle_qt_message(message)
                        except json.JSONDecodeError as e:
                            print(f"JSON解析错误: {e}")
                            
            except Exception as e:
                if self.running:
                    print(f"接收消息错误: {e}")
                break
    
    def handle_qt_message(self, message):
        """处理来自Qt的消息"""
        msg_type = message.get("type", "unknown")
        print(f"收到Qt消息: {msg_type}")
        
        if msg_type == "workpiece_data":
            # 模拟工件加载
            time.sleep(1)  # 模拟加载时间
            self.send_message("workpiece_loaded", {
                "success": True,
                "message": "工件加载成功"
            })
            
        elif msg_type == "trajectory_data":
            # 模拟轨迹显示
            time.sleep(0.5)
            self.send_message("trajectory_displayed", {
                "success": True,
                "message": "轨迹显示成功"
            })
            
        elif msg_type == "simulation_command":
            command = message.get("command", "")
            if command == "start":
                print("开始仿真...")
                # 模拟仿真过程
                threading.Timer(3.0, self.simulate_completion).start()
            elif command == "stop":
                print("停止仿真...")
            elif command == "reset":
                print("重置仿真...")
                
        elif msg_type == "camera_command":
            command = message.get("command", "")
            print(f"相机命令: {command}")
            
        elif msg_type == "heartbeat_response":
            # 心跳响应
            pass
    
    def simulate_completion(self):
        """模拟仿真完成"""
        self.send_message("simulation_complete", {
            "status": "completed",
            "duration": 3.0,
            "quality_score": 0.95
        })
    
    def simulate_collision(self):
        """模拟碰撞检测"""
        self.send_message("collision_detected", {
            "message": "检测到机械臂与工件碰撞",
            "position": {"x": 100, "y": 200, "z": 150},
            "severity": "high"
        })
    
    def run_test_sequence(self):
        """运行测试序列"""
        if not self.connect():
            return
            
        # 启动消息监听线程
        listen_thread = threading.Thread(target=self.listen_for_messages)
        listen_thread.daemon = True
        listen_thread.start()
        
        print("\n=== Unity通信测试开始 ===")
        
        try:
            # 等待Qt应用程序准备就绪
            time.sleep(2)
            
            # 测试序列
            print("\n1. 发送心跳...")
            self.send_message("heartbeat")
            time.sleep(1)
            
            print("\n2. 模拟工件加载...")
            # 这将由Qt发送，我们在这里等待
            
            print("\n3. 等待Qt命令...")
            print("请在Qt应用程序中:")
            print("- 点击'初始化Unity引擎'")
            print("- 尝试加载工件数据")
            print("- 启动仿真")
            print("- 按Ctrl+C退出测试")
            
            # 保持连接
            while self.running:
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\n测试被用户中断")
        finally:
            self.disconnect()

if __name__ == "__main__":
    client = UnityTestClient()
    client.run_test_sequence()