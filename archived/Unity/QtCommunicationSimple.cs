using System;
using System.Net.Sockets;
using System.Text;
using UnityEngine;
using System.Collections;

/// <summary>
/// 简化版Unity与Qt通信脚本
/// 专门用于测试连接
/// </summary>
public class QtCommunicationSimple : MonoBehaviour
{
    [Header("连接设置")]
    public string qtHost = "localhost";
    public int qtPort = 12345;
    
    [Header("状态显示")]
    public bool isConnected = false;
    public string connectionStatus = "未连接";
    
    private TcpClient tcpClient;
    private NetworkStream stream;
    
    void Start()
    {
        Debug.Log("Unity Qt通信模块启动");
        ConnectToQt();
    }
    
    void ConnectToQt()
    {
        try
        {
            connectionStatus = "正在连接...";
            Debug.Log("尝试连接到Qt应用程序 " + qtHost + ":" + qtPort);
            
            tcpClient = new TcpClient();
            tcpClient.Connect(qtHost, qtPort);
            stream = tcpClient.GetStream();
            isConnected = true;
            connectionStatus = "已连接";
            
            Debug.Log("✅ 成功连接到Qt应用程序");
            
            // 发送初始心跳
            SendHeartbeat();
        }
        catch (Exception e)
        {
            isConnected = false;
            connectionStatus = "连接失败: " + e.Message;
            Debug.LogError("连接Qt失败: " + e.Message);
        }
    }
    
    void SendMessage(string messageType, string data)
    {
        if (!isConnected || stream == null) return;
        
        try
        {
            // 简单的JSON格式
            string json = "{\"type\":\"" + messageType + "\",\"timestamp\":" + 
                         DateTimeOffset.UtcNow.ToUnixTimeSeconds() + 
                         (string.IsNullOrEmpty(data) ? "" : ",\"data\":" + data) + "}\n";
            
            byte[] bytes = Encoding.UTF8.GetBytes(json);
            stream.Write(bytes, 0, bytes.Length);
            
            Debug.Log("📤 发送消息: " + messageType);
        }
        catch (Exception e)
        {
            Debug.LogError("发送消息失败: " + e.Message);
        }
    }
    
    void SendHeartbeat()
    {
        SendMessage("heartbeat", null);
    }
    
    // 测试方法
    public void TestWorkpieceLoaded()
    {
        SendMessage("workpiece_loaded", "{\"success\":true,\"message\":\"Unity工件加载成功\"}");
    }
    
    public void TestTrajectoryDisplayed()
    {
        SendMessage("trajectory_displayed", "{\"success\":true,\"message\":\"Unity轨迹显示成功\"}");
    }
    
    public void TestSimulationComplete()
    {
        SendMessage("simulation_complete", "{\"status\":\"completed\",\"duration\":5.0,\"quality_score\":0.95}");
    }
    
    public void TestCollisionDetected()
    {
        SendMessage("collision_detected", "{\"message\":\"Unity检测到碰撞\",\"position\":{\"x\":100,\"y\":200,\"z\":150}}");
    }
    
    void OnDestroy()
    {
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
        // 在屏幕上显示连接状态和测试按钮
        GUI.Label(new Rect(10, 10, 300, 20), "Qt连接状态: " + connectionStatus);
        
        if (GUI.Button(new Rect(10, 40, 100, 30), "发送心跳"))
        {
            SendHeartbeat();
        }
        
        if (GUI.Button(new Rect(120, 40, 120, 30), "工件加载"))
        {
            TestWorkpieceLoaded();
        }
        
        if (GUI.Button(new Rect(250, 40, 120, 30), "轨迹显示"))
        {
            TestTrajectoryDisplayed();
        }
        
        if (GUI.Button(new Rect(10, 80, 120, 30), "仿真完成"))
        {
            TestSimulationComplete();
        }
        
        if (GUI.Button(new Rect(140, 80, 120, 30), "碰撞检测"))
        {
            TestCollisionDetected();
        }
        
        if (GUI.Button(new Rect(270, 80, 100, 30), "重新连接"))
        {
            ConnectToQt();
        }
    }
}