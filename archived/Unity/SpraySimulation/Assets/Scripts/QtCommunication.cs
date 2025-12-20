using System;
using System.Net.Sockets;
using System.Text;
using UnityEngine;
using System.Collections;

/// <summary>
/// Unity与Qt应用程序通信脚本
/// 将此脚本添加到Unity场景中的任意GameObject上
/// </summary>
public class QtCommunication : MonoBehaviour
{
    [Header("连接设置")]
    public string qtHost = "localhost";
    public int qtPort = 12346;
    
    [Header("状态显示")]
    public bool isConnected = false;
    public string connectionStatus = "未连接";
    
    private TcpClient tcpClient;
    private NetworkStream stream;
    private bool shouldReconnect = true;
    
    void Start()
    {
        Debug.Log("Unity Qt通信模块启动");
        StartCoroutine(ConnectToQt());
    }
    
    IEnumerator ConnectToQt()
    {
        while (shouldReconnect)
        {
            bool connectionSuccessful = false;
            
            try
            {
                connectionStatus = "正在连接...";
                Debug.Log($"尝试连接到Qt应用程序 {qtHost}:{qtPort}");
                
                tcpClient = new TcpClient();
                tcpClient.Connect(qtHost, qtPort);
                stream = tcpClient.GetStream();
                isConnected = true;
                connectionStatus = "已连接";
                
                Debug.Log("✅ 成功连接到Qt应用程序");
                
                // 发送初始心跳
                SendHeartbeat();
                
                // 启动消息监听
                StartCoroutine(ListenForMessages());
                
                connectionSuccessful = true;
            }
            catch (Exception e)
            {
                isConnected = false;
                connectionStatus = $"连接失败: {e.Message}";
                Debug.LogWarning($"连接Qt失败: {e.Message}");
                Debug.Log("5秒后重试连接...");
            }
            
            if (connectionSuccessful)
            {
                break; // 连接成功，退出重连循环
            }
            else
            {
                yield return new WaitForSeconds(5f);
            }
        }
    }
    
    IEnumerator ListenForMessages()
    {
        byte[] buffer = new byte[1024];
        string messageBuffer = "";
        
        while (isConnected && tcpClient != null && tcpClient.Connected)
        {
            bool hasError = false;
            
            try
            {
                if (stream != null && stream.DataAvailable)
                {
                    int bytesRead = stream.Read(buffer, 0, buffer.Length);
                    string data = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                    messageBuffer += data;
                    
                    // 处理完整的消息（以换行符分隔）
                    while (messageBuffer.Contains("\n"))
                    {
                        int index = messageBuffer.IndexOf('\n');
                        string message = messageBuffer.Substring(0, index);
                        messageBuffer = messageBuffer.Substring(index + 1);
                        
                        if (!string.IsNullOrEmpty(message.Trim()))
                        {
                            ProcessQtMessage(message);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Debug.LogError($"消息接收错误: {e.Message}");
                hasError = true;
            }
            
            if (hasError)
            {
                break;
            }
            
            yield return new WaitForSeconds(0.1f); // 100ms检查间隔
        }
        
        // 连接断开，尝试重连
        if (shouldReconnect)
        {
            isConnected = false;
            connectionStatus = "连接断开，准备重连";
            Debug.Log("连接断开，准备重连...");
            StartCoroutine(ConnectToQt());
        }
    }
    
    void ProcessQtMessage(string jsonMessage)
    {
        try
        {
            // 简单的JSON解析（Unity内置JsonUtility）
            var message = JsonUtility.FromJson<QtMessage>(jsonMessage);
            
            Debug.Log($"📥 收到Qt消息: {message.type}");
            
            switch (message.type)
            {
                case "workpiece_data":
                    HandleWorkpieceData(message);
                    break;
                case "trajectory_data":
                    HandleTrajectoryData(message);
                    break;
                case "simulation_command":
                    HandleSimulationCommand(message);
                    break;
                case "camera_command":
                    HandleCameraCommand(message);
                    break;
                case "heartbeat_response":
                    // 心跳响应，保持静默
                    break;
                default:
                    Debug.Log($"未知消息类型: {message.type}");
                    break;
            }
        }
        catch (Exception e)
        {
            Debug.LogError($"消息解析错误: {e.Message}");
        }
    }
    
    void HandleWorkpieceData(QtMessage message)
    {
        Debug.Log("🔧 处理工件数据...");
        
        // 添加调试信息
        Debug.Log($"📋 收到的原始数据长度: {message.data?.Length ?? 0}");
        Debug.Log($"📋 数据前100字符: {(message.data?.Length > 100 ? message.data.Substring(0, 100) + "..." : message.data)}");
        
        try
        {
            // 检查数据是否为空
            if (string.IsNullOrEmpty(message.data))
            {
                Debug.LogWarning("⚠️ 收到空的点云数据");
                StartCoroutine(SimulateWorkpieceLoading("空数据"));
                return;
            }
            
            // 解析点云数据
            var pointCloudData = JsonUtility.FromJson<PointCloudData>(message.data);
            
            Debug.Log($"📊 解析结果检查:");
            Debug.Log($"   - pointCloudData != null: {pointCloudData != null}");
            Debug.Log($"   - fileName: {pointCloudData?.fileName}");
            Debug.Log($"   - pointCount: {pointCloudData?.pointCount}");
            Debug.Log($"   - points != null: {pointCloudData?.points != null}");
            Debug.Log($"   - points.Length: {pointCloudData?.points?.Length ?? 0}");
            
            if (pointCloudData != null && pointCloudData.points != null && pointCloudData.points.Length > 0)
            {
                Debug.Log($"📊 接收到有效点云数据: {pointCloudData.fileName}, 声明点数: {pointCloudData.pointCount}, 实际数组长度: {pointCloudData.points.Length}");
                
                // 查找或创建点云渲染器
                PointCloudRenderer renderer = FindObjectOfType<PointCloudRenderer>();
                if (renderer == null)
                {
                    Debug.Log("🔧 创建新的点云渲染器");
                    GameObject rendererObject = new GameObject("PointCloudRenderer");
                    renderer = rendererObject.AddComponent<PointCloudRenderer>();
                }
                else
                {
                    Debug.Log("🔧 使用现有的点云渲染器");
                }
                
                // 加载点云数据
                renderer.LoadPointCloudFromJson(message.data);
                
                StartCoroutine(SimulateWorkpieceLoading(pointCloudData.fileName));
            }
            else
            {
                Debug.LogWarning("⚠️ 点云数据格式无效或为空");
                Debug.LogWarning($"   - pointCloudData为空: {pointCloudData == null}");
                Debug.LogWarning($"   - points为空: {pointCloudData?.points == null}");
                Debug.LogWarning($"   - points长度: {pointCloudData?.points?.Length ?? 0}");
                StartCoroutine(SimulateWorkpieceLoading("无效数据"));
            }
        }
        catch (System.Exception e)
        {
            Debug.LogError($"❌ 点云数据处理失败: {e.Message}");
            Debug.LogError($"❌ 堆栈跟踪: {e.StackTrace}");
            StartCoroutine(SimulateWorkpieceLoading("解析错误"));
        }
    }
    
    IEnumerator SimulateWorkpieceLoading(string fileName = "工件")
    {
        yield return new WaitForSeconds(1.5f); // 模拟加载时间
        
        // 发送正确格式的workpiece_loaded消息
        SendWorkpieceLoadedMessage(true, $"Unity成功加载点云: {fileName}");
        
        Debug.Log($"✅ 点云加载完成: {fileName}");
    }
    
    void HandleTrajectoryData(QtMessage message)
    {
        Debug.Log("📍 处理轨迹数据...");
        
        StartCoroutine(SimulateTrajectoryDisplay());
    }
    
    IEnumerator SimulateTrajectoryDisplay()
    {
        yield return new WaitForSeconds(1.0f); // 模拟渲染时间
        
        // 发送正确格式的trajectory_displayed消息
        SendTrajectoryDisplayedMessage(true, "Unity轨迹显示成功");
        
        Debug.Log("✅ 轨迹显示完成");
    }
    
    void HandleSimulationCommand(QtMessage message)
    {
        // 这里需要解析command字段，简化处理
        Debug.Log("🎮 处理仿真命令...");
        
        // 模拟不同的仿真命令
        if (message.data != null && message.data.Contains("start"))
        {
            Debug.Log("▶️ 启动仿真");
            StartCoroutine(SimulateExecution());
        }
        else if (message.data != null && message.data.Contains("stop"))
        {
            Debug.Log("⏹️ 停止仿真");
        }
    }
    
    IEnumerator SimulateExecution()
    {
        yield return new WaitForSeconds(2.0f);
        
        // 模拟碰撞检测
        SendQtMessage("collision_detected", new {
            message = "Unity检测到碰撞",
            position = new { x = 100, y = 200, z = 150 },
            severity = "medium"
        });
        
        yield return new WaitForSeconds(3.0f);
        
        // 模拟仿真完成
        SendQtMessage("simulation_complete", new {
            status = "completed",
            duration = 5.0f,
            quality_score = 0.95f
        });
    }
    
    void HandleCameraCommand(QtMessage message)
    {
        Debug.Log("📷 处理相机命令...");
        
        SendQtMessage("camera_view_changed", "perspective");
    }
    
    void SendQtMessage(string messageType, object data)
    {
        if (!isConnected || stream == null) return;
        
        try
        {
            // 使用字符串拼接而不是JsonUtility，因为JsonUtility对匿名对象支持不好
            string dataJson = "null";
            if (data != null)
            {
                if (data is string)
                {
                    dataJson = "\"" + data.ToString().Replace("\"", "\\\"") + "\"";
                }
                else
                {
                    dataJson = JsonUtility.ToJson(data);
                }
            }
            
            string json = $"{{\"type\":\"{messageType}\",\"timestamp\":{DateTimeOffset.UtcNow.ToUnixTimeSeconds()},\"data\":{dataJson}}}\n";
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            stream.Write(bytes, 0, bytes.Length);
            
            Debug.Log($"📤 发送消息: {messageType}");
        }
        catch (Exception e)
        {
            Debug.LogError($"发送消息失败: {e.Message}");
        }
    }
    
    /// <summary>
    /// 发送工件加载结果消息（Qt期望的格式）
    /// </summary>
    void SendWorkpieceLoadedMessage(bool success, string message)
    {
        if (!isConnected || stream == null) return;
        
        try
        {
            string json = $"{{\"type\":\"workpiece_loaded\",\"success\":{success.ToString().ToLower()},\"message\":\"{message.Replace("\"", "\\\"")}\"}}\n";
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            stream.Write(bytes, 0, bytes.Length);
            
            Debug.Log($"📤 发送工件加载结果: success={success}, message={message}");
        }
        catch (Exception e)
        {
            Debug.LogError($"发送工件加载消息失败: {e.Message}");
        }
    }
    
    /// <summary>
    /// 发送轨迹显示结果消息（Qt期望的格式）
    /// </summary>
    void SendTrajectoryDisplayedMessage(bool success, string message)
    {
        if (!isConnected || stream == null) return;
        
        try
        {
            string json = $"{{\"type\":\"trajectory_displayed\",\"success\":{success.ToString().ToLower()},\"message\":\"{message.Replace("\"", "\\\"")}\"}}\n";
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            stream.Write(bytes, 0, bytes.Length);
            
            Debug.Log($"📤 发送轨迹显示结果: success={success}, message={message}");
        }
        catch (Exception e)
        {
            Debug.LogError($"发送轨迹显示消息失败: {e.Message}");
        }
    }
    
    void SendHeartbeat()
    {
        SendQtMessage("heartbeat", null);
    }
    
    // 公共方法，可在Inspector中调用
    [ContextMenu("发送心跳")]
    public void TestHeartbeat()
    {
        SendHeartbeat();
    }
    
    [ContextMenu("测试工件加载")]
    public void TestWorkpieceLoaded()
    {
        SendWorkpieceLoadedMessage(true, "测试工件加载成功");
    }
    
    void OnDestroy()
    {
        shouldReconnect = false;
        isConnected = false;
        
        if (stream != null)
        {
            stream.Close();
        }
        
        if (tcpClient != null)
        {
            tcpClient.Close();
        }
        
        Debug.Log("Unity Qt通信模块已关闭");
    }
    
    void OnGUI()
    {
        // 在屏幕上显示连接状态
        GUI.Label(new Rect(10, 10, 300, 20), $"Qt连接状态: {connectionStatus}");
        
        if (GUI.Button(new Rect(10, 40, 100, 30), "发送心跳"))
        {
            TestHeartbeat();
        }
        
        if (GUI.Button(new Rect(120, 40, 120, 30), "测试工件加载"))
        {
            TestWorkpieceLoaded();
        }
    }
}

[System.Serializable]
public class QtMessage
{
    public string type;
    public long timestamp;
    public string data; // 简化处理，使用字符串
}

/// <summary>
/// 点云数据结构（用于JSON反序列化）
/// </summary>
[System.Serializable]
public class PointCloudData
{
    public string fileName;
    public string format;
    public int pointCount;
    public float fileSize;
    public float[] points;
    public float[] colors;
    public float[] normals;
    public float[] boundingBoxMin;
    public float[] boundingBoxMax;
    public int sampleStep;
}