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
    public float distance = 100f;
    public float minDistance = 1f; // 大幅减少最小距离，允许更近距离观察
    public float maxDistance = 50000f; // 进一步增加最大距离
    public float zoomSpeed = 1000f; // 进一步增加基础缩放速度
    public bool adaptiveZoomSpeed = true; // 自适应缩放速度
    public float zoomAcceleration = 2.5f; // 增加缩放加速度
    
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
        // 强制设置距离限制，确保Inspector设置生效
        minDistance = 1f;
        maxDistance = 50000f;
        
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
        Debug.Log($"📷 距离限制: 最小={minDistance:F1}, 最大={maxDistance:F0}");
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
        
        // 快捷键调整相机距离
        if (Input.GetKeyDown(KeyCode.F))
        {
            // F键：快速拉近到合适距离
            Vector3 size = currentTargetPosition != Vector3.zero ? 
                new Vector3(1000, 1000, 1000) : new Vector3(100, 100, 100); // 估算大小
            targetDistance = size.magnitude * 2.0f;
            targetDistance = Mathf.Clamp(targetDistance, minDistance, maxDistance);
            Debug.Log($"📷 快速聚焦: 目标距离={targetDistance:F0}");
        }
        else if (Input.GetKeyDown(KeyCode.G))
        {
            // G键：拉远到全景视角
            Vector3 size = currentTargetPosition != Vector3.zero ? 
                new Vector3(1000, 1000, 1000) : new Vector3(100, 100, 100); // 估算大小
            targetDistance = size.magnitude * 5.0f;
            targetDistance = Mathf.Clamp(targetDistance, minDistance, maxDistance);
            Debug.Log($"📷 全景视角: 目标距离={targetDistance:F0}");
        }
        
        // 鼠标滚轮缩放 - 大幅优化的自适应速度
        float scroll = Input.GetAxis("Mouse ScrollWheel");
        if (scroll != 0f)
        {
            float oldDistance = targetDistance;
            
            if (adaptiveZoomSpeed)
            {
                // 使用更激进的指数缩放算法，大幅提高速度
                float baseSpeed = zoomSpeed;
                
                // 根据当前距离计算缩放速度（更激进的速度增长）
                if (distance < 50f)
                {
                    // 极近距离：精细缩放
                    baseSpeed = zoomSpeed * 0.1f;
                }
                else if (distance < 200f)
                {
                    // 近距离：较快缩放
                    baseSpeed = zoomSpeed * 0.5f;
                }
                else if (distance < 1000f)
                {
                    // 中距离：快速缩放
                    baseSpeed = zoomSpeed * (distance / 100f);
                }
                else if (distance < 5000f)
                {
                    // 远距离：很快缩放
                    baseSpeed = zoomSpeed * (distance / 20f);
                }
                else
                {
                    // 极远距离：超快缩放
                    baseSpeed = zoomSpeed * (distance / 5f);
                }
                
                // 提高速度上限
                baseSpeed = Mathf.Clamp(baseSpeed, zoomSpeed * 0.1f, zoomSpeed * 500f); // 从100f提高到500f
                
                targetDistance -= scroll * baseSpeed;
                
                Debug.Log($"🔍 自适应缩放: 当前距离={distance:F0}, 基础速度={baseSpeed:F0}, 滚轮={scroll:F3}");
            }
            else
            {
                // 固定速度缩放
                targetDistance -= scroll * zoomSpeed;
            }
            
            // 确保距离在合理范围内
            float clampedDistance = Mathf.Clamp(targetDistance, minDistance, maxDistance);
            
            Debug.Log($"🔍 缩放详情: {oldDistance:F0} → {targetDistance:F0} → {clampedDistance:F0}");
            Debug.Log($"🔍 限制范围: 最小={minDistance:F1}, 最大={maxDistance:F0}");
            
            if (targetDistance != clampedDistance)
            {
                Debug.LogWarning($"⚠️ 距离被限制: 目标={targetDistance:F0}, 实际={clampedDistance:F0}");
            }
            
            targetDistance = clampedDistance;
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
        float newDistance = size * 3.0f; // 减少到3倍大小，更近距离观察
        
        // 确保距离在合理范围内，优先近距离观察
        newDistance = Mathf.Clamp(newDistance, size * 1.5f, maxDistance);
        
        // 强制设置初始角度（从理想的观察位置）
        currentX = 45f; // 水平角度：东北方向
        currentY = 25f; // 垂直角度：稍微向下看
        
        // 更新目标位置和距离
        currentTargetPosition = center;
        targetDistance = newDistance;
        distance = newDistance; // 立即设置，不使用平滑
        
        // 强制更新相机位置
        UpdateCameraImmediate();
        
        Debug.Log($"📷 强制聚焦到边界框: 中心={center}, 尺寸={bounds.size}, 距离={newDistance:F1}");
        Debug.Log($"📷 强制设置角度: 水平={currentX}°, 垂直={currentY}°");
    }
    
    /// <summary>
    /// 立即更新相机位置（不使用平滑）
    /// </summary>
    private void UpdateCameraImmediate()
    {
        // 计算旋转
        Quaternion rotation = Quaternion.Euler(currentY, currentX, 0);
        
        // 计算位置
        Vector3 direction = rotation * Vector3.back;
        Vector3 targetPos = currentTargetPosition + direction * distance;
        
        // 立即应用变换
        transform.position = targetPos;
        transform.rotation = rotation;
        
        Debug.Log($"📷 立即更新相机: 位置={targetPos}, 旋转={rotation.eulerAngles}");
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
        GUI.Label(new Rect(10, Screen.height - 160, 300, 140), 
            "相机控制:\n" +
            "• 鼠标左键: 旋转\n" +
            "• 滚轮: 缩放\n" +
            "• Shift+左键: 平移\n" +
            "• R键: 重置\n" +
            "• F键: 近距离聚焦\n" +
            "• G键: 全景视角");
    }
}