using UnityEngine;
using System.Net.Sockets;
using System.Text;
using System;

public class QtTestBasic : MonoBehaviour
{
    public bool isConnected = false;
    private TcpClient client;
    private NetworkStream stream;
    
    void Start()
    {
        Debug.Log("开始连接Qt...");
        ConnectToQt();
    }
    
    void ConnectToQt()
    {
        try
        {
            client = new TcpClient("localhost", 12345);
            stream = client.GetStream();
            isConnected = true;
            Debug.Log("连接Qt成功！");
            
            // 发送测试消息
            SendTestMessage();
        }
        catch (Exception e)
        {
            Debug.LogError("连接失败: " + e.Message);
        }
    }
    
    void SendTestMessage()
    {
        if (stream != null)
        {
            string message = "{\"type\":\"heartbeat\"}\n";
            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log("发送心跳消息");
        }
    }
    
    void OnDestroy()
    {
        if (stream != null) stream.Close();
        if (client != null) client.Close();
    }
    
    void OnGUI()
    {
        GUI.Label(new Rect(10, 10, 200, 20), "连接状态: " + (isConnected ? "已连接" : "未连接"));
        
        if (GUI.Button(new Rect(10, 40, 100, 30), "发送心跳"))
        {
            SendTestMessage();
        }
    }
}