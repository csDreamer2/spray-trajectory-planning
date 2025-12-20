using UnityEngine;

/// <summary>
/// 轨道相机控制器
/// 提供鼠标控制的相机旋转、缩放和平移功能
/// </summary>
public class OrbitCameraController : MonoBehaviour
{
    [Header("目标设置")]
    public Transform target; // 相机围绕的目标点
    public Vector3 targetPosition = Vector3.zero; // 如果没有target，使用这个位置
    
    [Header("距离控制")]
    public float distance = 10f;
    public float minDistance = 2f;
    public float maxDistance = 50f;
    public float zoomSpeed = 2f;
    
    [Header("旋转控制")]
    public float rotationSpeed = 2f;
    public float minVerticalAngle = -80f;
    public float maxVerticalAngle = 80f;
    
    [Header("平移控制")]
    public float panSpeed = 1f;
    
    [Header("平滑设置")]
    public float smoothTime = 0.1f;
    public bool enableSmoothing = true;
    
    [Header("输入设置")]
    public KeyCode panKey = KeyCode.LeftShift;
    public KeyCode resetKey = KeyCode.R;
    
    // 私有变量
    private float currentX = 0f;
    private float currentY = 0f;
    private float targetDistance;
    private Vector3 currentTargetPosition;
    
    // 平滑变量
    private Vector3 velocityPosition = Vector3.zero;
    private float velocityDistance = 0f;
    
    // 初始状态
    private Vector3 initialPosition;
    private Quaternion initialRotation;
    private Vector3 initialTargetPosition;
    
    void Start()
    {
        // 保存初始状态
        initialPosition = transform.position;
        initialRotation = transform.rotation;
        initialTargetPosition = target != null ? target.position : targetPosition;
        
        // 初始化当前目标位置
        currentTargetPosition = initialTargetPosition;
        
        // 计算初始角度和距离
        Vector3 angles = transform.eulerAngles;
        currentX = angles.y;
        currentY = angles.x;
        
        // 计算初始距离
        Vector3 targetPos = target != null ? target.position : targetPosition;
        targetDistance = Vector3.Distance(transform.position, targetPos);
        distance = targetDistance;
        
        Debug.Log($"📷 轨道相机控制器初始化 - 距离: {distance:F2}, 角度: ({currentX:F1}, {currentY:F1})");
    }
    
    void Update()
    {
        HandleInput();
        UpdateCamera();
    }
    
    void HandleInput()
    {
        // 重置相机
        if (Input.GetKeyDown(resetKey))
        {
            ResetCamera();
            return;
        }
        
        // 鼠标滚轮缩放
        float scroll = Input.GetAxis("Mouse ScrollWheel");
        if (scroll != 0f)
        {
            targetDistance -= scroll * zoomSpeed;
            targetDistance = Mathf.Clamp(targetDistance, minDistance, maxDistance);
        }
        
        // 鼠标左键旋转
        if (Input.GetMouseButton(0) && !Input.GetKey(panKey))
        {
            currentX += Input.GetAxis("Mouse X") * rotationSpeed;
            currentY -= Input.GetAxis("Mouse Y") * rotationSpeed;
            currentY = Mathf.Clamp(currentY, minVerticalAngle, maxVerticalAngle);
        }
        
        // Shift + 鼠标左键平移
        if (Input.GetMouseButton(0) && Input.GetKey(panKey))
        {
            Vector3 panMovement = new Vector3(-Input.GetAxis("Mouse X"), -Input.GetAxis("Mouse Y"), 0) * panSpeed * (distance / 10f);
            panMovement = transform.TransformDirection(panMovement);
            currentTargetPosition += panMovement;
        }
        
        // 鼠标中键平移
        if (Input.GetMouseButton(2))
        {
            Vector3 panMovement = new Vector3(-Input.GetAxis("Mouse X"), -Input.GetAxis("Mouse Y"), 0) * panSpeed * (distance / 10f);
            panMovement = transform.TransformDirection(panMovement);
            currentTargetPosition += panMovement;
        }
    }
    
    void UpdateCamera()
    {
        // 更新目标位置（如果有target对象）
        if (target != null)
        {
            currentTargetPosition = target.position;
        }
        
        // 平滑距离变化
        if (enableSmoothing)
        {
            distance = Mathf.SmoothDamp(distance, targetDistance, ref velocityDistance, smoothTime);
        }
        else
        {
            distance = targetDistance;
        }
        
        // 计算旋转
        Quaternion rotation = Quaternion.Euler(currentY, currentX, 0);
        
        // 计算位置
        Vector3 direction = rotation * Vector3.back;
        Vector3 targetPos = currentTargetPosition + direction * distance;
        
        // 应用变换
        if (enableSmoothing)
        {
            transform.position = Vector3.SmoothDamp(transform.position, targetPos, ref velocityPosition, smoothTime);
        }
        else
        {
            transform.position = targetPos;
        }
        
        transform.rotation = rotation;
    }
    
    /// <summary>
    /// 重置相机到初始状态
    /// </summary>
    public void ResetCamera()
    {
        currentX = initialRotation.eulerAngles.y;
        currentY = initialRotation.eulerAngles.x;
        targetDistance = Vector3.Distance(initialPosition, initialTargetPosition);
        currentTargetPosition = initialTargetPosition;
        
        Debug.Log("📷 相机已重置到初始状态");
    }
    
    /// <summary>
    /// 聚焦到指定位置
    /// </summary>
    public void FocusOn(Vector3 position, float newDistance = -1f)
    {
        currentTargetPosition = position;
        if (newDistance > 0)
        {
            targetDistance = Mathf.Clamp(newDistance, minDistance, maxDistance);
        }
        
        Debug.Log($"📷 相机聚焦到: {position}, 距离: {targetDistance:F2}");
    }
    
    /// <summary>
    /// 聚焦到边界框
    /// </summary>
    public void FocusOnBounds(Bounds bounds)
    {
        Vector3 center = bounds.center;
        float size = bounds.size.magnitude;
        float newDistance = size * 1.5f; // 1.5倍大小作为距离
        
        FocusOn(center, newDistance);
    }
    
    /// <summary>
    /// 设置旋转角度
    /// </summary>
    public void SetRotation(float horizontal, float vertical)
    {
        currentX = horizontal;
        currentY = Mathf.Clamp(vertical, minVerticalAngle, maxVerticalAngle);
    }
    
    /// <summary>
    /// 设置距离
    /// </summary>
    public void SetDistance(float newDistance)
    {
        targetDistance = Mathf.Clamp(newDistance, minDistance, maxDistance);
    }
    
    /// <summary>
    /// 获取当前状态信息
    /// </summary>
    public string GetCameraInfo()
    {
        return $"位置: {transform.position:F2}\n" +
               $"目标: {currentTargetPosition:F2}\n" +
               $"距离: {distance:F2}\n" +
               $"角度: ({currentX:F1}°, {currentY:F1}°)";
    }
    
    void OnGUI()
    {
        // 显示控制提示
        GUI.Label(new Rect(10, Screen.height - 120, 300, 100), 
            "相机控制:\n" +
            "• 鼠标左键: 旋转\n" +
            "• 滚轮: 缩放\n" +
            "• Shift+左键: 平移\n" +
            "• R键: 重置");
    }
}