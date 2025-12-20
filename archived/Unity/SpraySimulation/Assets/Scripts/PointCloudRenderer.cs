using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

/// <summary>
/// Unity点云渲染器
/// 负责接收Qt发送的点云数据并在Unity中进行可视化
/// </summary>
public class PointCloudRenderer : MonoBehaviour
{
    [Header("点云渲染设置")]
    public Material pointMaterial;
    public float pointSize = 5.0f; // 适中的点大小，根据工件尺寸自动调整
    public Color pointColor = Color.white;
    public bool useVertexColors = false;
    public float pointSizeMultiplier = 1.0f; // 运行时调整球体大小的倍数
    
    [Header("性能设置")]
    public int maxPointsPerMesh = 65000; // Unity网格顶点限制
    public bool enableLOD = true;
    public float[] lodDistances = { 10f, 50f, 100f };
    public int[] lodPointCounts = { 10000, 5000, 1000 };
    
    [Header("显示控制")]
    public bool showBoundingBox = true;
    public Color boundingBoxColor = Color.yellow;
    
    private List<GameObject> pointCloudMeshes = new List<GameObject>();
    private GameObject boundingBoxObject;
    private Camera mainCamera;
    
    // 点云数据
    private Vector3[] originalPoints;
    private Color[] originalColors;
    private Vector3 boundingBoxMin;
    private Vector3 boundingBoxMax;
    private string currentFileName;
    
    void Start()
    {
        mainCamera = Camera.main;
        if (mainCamera == null)
        {
            mainCamera = FindObjectOfType<Camera>();
        }
        
        // 创建默认材质
        if (pointMaterial == null)
        {
            pointMaterial = CreateDefaultPointMaterial();
        }
    }
    
    void Update()
    {
        if (enableLOD && originalPoints != null && originalPoints.Length > 0)
        {
            UpdateLOD();
        }
        
        // 运行时调整球体大小
        HandlePointSizeInput();
    }
    
    /// <summary>
    /// 处理球体大小调整输入
    /// </summary>
    private void HandlePointSizeInput()
    {
        if (Input.GetKeyDown(KeyCode.Equals) || Input.GetKeyDown(KeyCode.Plus))
        {
            // 增大球体
            pointSizeMultiplier = Mathf.Min(pointSizeMultiplier * 1.2f, 5.0f);
            UpdatePointSizes();
            Debug.Log($"🔵 增大球体大小: {pointSizeMultiplier:F2}");
        }
        else if (Input.GetKeyDown(KeyCode.Minus))
        {
            // 减小球体
            pointSizeMultiplier = Mathf.Max(pointSizeMultiplier * 0.8f, 0.1f);
            UpdatePointSizes();
            Debug.Log($"🔵 减小球体大小: {pointSizeMultiplier:F2}");
        }
        else if (Input.GetKeyDown(KeyCode.R) && Input.GetKey(KeyCode.LeftShift))
        {
            // 重置球体大小
            pointSizeMultiplier = 1.0f;
            UpdatePointSizes();
            Debug.Log($"🔵 重置球体大小: {pointSizeMultiplier:F2}");
        }
    }
    
    /// <summary>
    /// 更新所有球体的大小
    /// </summary>
    private void UpdatePointSizes()
    {
        foreach (GameObject meshObject in pointCloudMeshes)
        {
            if (meshObject != null && meshObject.name == "PointCloudSpheres")
            {
                for (int i = 0; i < meshObject.transform.childCount; i++)
                {
                    Transform child = meshObject.transform.GetChild(i);
                    if (child != null)
                    {
                        // 获取原始大小并应用倍数
                        Vector3 originalScale = Vector3.one * (pointSize / 5.0f); // 假设原始pointSize为5
                        child.localScale = originalScale * pointSizeMultiplier;
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// 加载点云数据
    /// </summary>
    public void LoadPointCloud(Vector3[] points, Color[] colors = null, Vector3 bboxMin = default, Vector3 bboxMax = default, string fileName = "")
    {
        Debug.Log($"🔧 开始加载点云: {fileName}, 点数: {points.Length}");
        Debug.Log($"📏 边界框: Min({bboxMin.x:F2}, {bboxMin.y:F2}, {bboxMin.z:F2}) Max({bboxMax.x:F2}, {bboxMax.y:F2}, {bboxMax.z:F2})");
        
        Vector3 size = bboxMax - bboxMin;
        Debug.Log($"📐 工件尺寸: {size.x:F2} × {size.y:F2} × {size.z:F2}");
        
        // 如果边界框无效，从点云数据计算
        if (size.magnitude < 0.1f)
        {
            Debug.Log("⚠️ 边界框无效，从点云数据重新计算...");
            Vector3 min = points[0];
            Vector3 max = points[0];
            
            foreach (Vector3 point in points)
            {
                min = Vector3.Min(min, point);
                max = Vector3.Max(max, point);
            }
            
            bboxMin = min;
            bboxMax = max;
            boundingBoxMin = min;
            boundingBoxMax = max;
            
            size = max - min;
            Debug.Log($"📏 重新计算的边界框: Min({min.x:F2}, {min.y:F2}, {min.z:F2}) Max({max.x:F2}, {max.y:F2}, {max.z:F2})");
            Debug.Log($"📐 重新计算的尺寸: {size.x:F2} × {size.y:F2} × {size.z:F2}");
        }
        
        // 清除之前的点云
        ClearPointCloud();
        
        // 保存原始数据
        originalPoints = points;
        originalColors = colors;
        boundingBoxMin = bboxMin;
        boundingBoxMax = bboxMax;
        currentFileName = fileName;
        
        // 创建点云网格（使用球体代替点）
        CreatePointCloudSpheres(points, colors);
        
        // 创建边界框
        if (showBoundingBox && bboxMin != bboxMax)
        {
            CreateBoundingBox(bboxMin, bboxMax);
        }
        
        // 调整相机视角
        FocusCamera();
        
        // 创建临时标记球体（用于调试）
        CreateDebugMarker();
        
        // 强制检查渲染状态
        CheckRenderingStatus();
        
        Debug.Log($"✅ 点云加载完成: {fileName}");
    }
    
    /// <summary>
    /// 从JSON数据加载点云（优化版本）
    /// </summary>
    public void LoadPointCloudFromJson(string jsonData)
    {
        try
        {
            Debug.Log($"🔄 开始解析JSON点云数据，大小: {jsonData.Length} 字符");
            
            var data = JsonUtility.FromJson<PointCloudData>(jsonData);
            
            if (data == null || data.points == null || data.points.Length == 0)
            {
                Debug.LogError("❌ JSON数据无效或为空");
                return;
            }
            
            Debug.Log($"📊 JSON解析成功 - 文件: {data.fileName}, 点数组长度: {data.points.Length}");
            
            // 预分配内存，提高性能
            int expectedPointCount = data.points.Length / 3;
            List<Vector3> points = new List<Vector3>(expectedPointCount);
            List<Color> colors = new List<Color>(expectedPointCount);
            
            // 批量解析点数据，提高效率
            for (int i = 0; i < data.points.Length - 2; i += 3)
            {
                Vector3 point = new Vector3(data.points[i], data.points[i + 1], data.points[i + 2]);
                
                // 验证点的有效性
                if (IsValidPoint(point))
                {
                    points.Add(point);
                    
                    // 处理颜色数据
                    if (data.colors != null && i + 2 < data.colors.Length)
                    {
                        Color color = new Color(data.colors[i], data.colors[i + 1], data.colors[i + 2], 1.0f);
                        colors.Add(color);
                    }
                    else
                    {
                        colors.Add(pointColor);
                    }
                }
            }
            
            Debug.Log($"📈 有效点数: {points.Count}/{expectedPointCount}");
            
            // 解析边界框
            Vector3 bboxMin = Vector3.zero;
            Vector3 bboxMax = Vector3.zero;
            
            if (data.boundingBoxMin != null && data.boundingBoxMin.Length >= 3)
            {
                bboxMin = new Vector3(data.boundingBoxMin[0], data.boundingBoxMin[1], data.boundingBoxMin[2]);
            }
            
            if (data.boundingBoxMax != null && data.boundingBoxMax.Length >= 3)
            {
                bboxMax = new Vector3(data.boundingBoxMax[0], data.boundingBoxMax[1], data.boundingBoxMax[2]);
            }
            
            Debug.Log($"📦 边界框: Min{bboxMin} Max{bboxMax}");
            
            // 加载点云
            LoadPointCloud(points.ToArray(), colors.ToArray(), bboxMin, bboxMax, data.fileName);
        }
        catch (System.Exception e)
        {
            Debug.LogError($"❌ 点云JSON解析失败: {e.Message}\n堆栈: {e.StackTrace}");
        }
    }
    
    /// <summary>
    /// 验证点的有效性
    /// </summary>
    private bool IsValidPoint(Vector3 point)
    {
        return !float.IsNaN(point.x) && !float.IsNaN(point.y) && !float.IsNaN(point.z) &&
               !float.IsInfinity(point.x) && !float.IsInfinity(point.y) && !float.IsInfinity(point.z);
    }
    
    /// <summary>
    /// 创建点云网格
    /// </summary>
    private void CreatePointCloudMeshes(Vector3[] points, Color[] colors)
    {
        int totalPoints = points.Length;
        int meshCount = Mathf.CeilToInt((float)totalPoints / maxPointsPerMesh);
        
        for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
        {
            int startIndex = meshIndex * maxPointsPerMesh;
            int endIndex = Mathf.Min(startIndex + maxPointsPerMesh, totalPoints);
            int pointCount = endIndex - startIndex;
            
            // 创建网格对象
            GameObject meshObject = new GameObject($"PointCloudMesh_{meshIndex}");
            meshObject.transform.SetParent(transform);
            
            MeshFilter meshFilter = meshObject.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = meshObject.AddComponent<MeshRenderer>();
            
            // 创建网格
            Mesh mesh = new Mesh();
            mesh.name = $"PointCloud_{meshIndex}";
            
            // 顶点数据
            Vector3[] vertices = new Vector3[pointCount];
            Color[] vertexColors = new Color[pointCount];
            int[] indices = new int[pointCount];
            
            for (int i = 0; i < pointCount; i++)
            {
                vertices[i] = points[startIndex + i];
                vertexColors[i] = (colors != null && startIndex + i < colors.Length) ? colors[startIndex + i] : pointColor;
                indices[i] = i;
            }
            
            // 暂时改用线段渲染，更容易看见
            mesh.vertices = vertices;
            mesh.colors = vertexColors;
            mesh.SetIndices(indices, MeshTopology.Lines, 0);
            mesh.RecalculateBounds();
            
            Debug.Log($"🔧 网格设置: 顶点={vertices.Length}, 索引={indices.Length}, 拓扑=Lines");
            
            meshFilter.mesh = mesh;
            meshRenderer.material = pointMaterial;
            
            pointCloudMeshes.Add(meshObject);
        }
        
        Debug.Log($"📊 创建了 {meshCount} 个点云网格，总点数: {totalPoints}");
    }
    
    /// <summary>
    /// 创建边界框
    /// </summary>
    private void CreateBoundingBox(Vector3 min, Vector3 max)
    {
        boundingBoxObject = new GameObject("BoundingBox");
        boundingBoxObject.transform.SetParent(transform);
        
        LineRenderer lineRenderer = boundingBoxObject.AddComponent<LineRenderer>();
        lineRenderer.material = CreateLineMaterial(boundingBoxColor);
        lineRenderer.startWidth = 0.02f;
        lineRenderer.endWidth = 0.02f;
        lineRenderer.useWorldSpace = false;
        
        // 边界框的12条边
        Vector3[] boxVertices = new Vector3[]
        {
            // 底面
            new Vector3(min.x, min.y, min.z), new Vector3(max.x, min.y, min.z),
            new Vector3(max.x, min.y, min.z), new Vector3(max.x, min.y, max.z),
            new Vector3(max.x, min.y, max.z), new Vector3(min.x, min.y, max.z),
            new Vector3(min.x, min.y, max.z), new Vector3(min.x, min.y, min.z),
            
            // 顶面
            new Vector3(min.x, max.y, min.z), new Vector3(max.x, max.y, min.z),
            new Vector3(max.x, max.y, min.z), new Vector3(max.x, max.y, max.z),
            new Vector3(max.x, max.y, max.z), new Vector3(min.x, max.y, max.z),
            new Vector3(min.x, max.y, max.z), new Vector3(min.x, max.y, min.z),
            
            // 垂直边
            new Vector3(min.x, min.y, min.z), new Vector3(min.x, max.y, min.z),
            new Vector3(max.x, min.y, min.z), new Vector3(max.x, max.y, min.z),
            new Vector3(max.x, min.y, max.z), new Vector3(max.x, max.y, max.z),
            new Vector3(min.x, min.y, max.z), new Vector3(min.x, max.y, max.z)
        };
        
        lineRenderer.positionCount = boxVertices.Length;
        lineRenderer.SetPositions(boxVertices);
    }
    
    /// <summary>
    /// 调整相机视角以适应点云
    /// </summary>
    private void FocusCamera()
    {
        if (mainCamera == null || boundingBoxMin == boundingBoxMax) return;
        
        Vector3 center = (boundingBoxMin + boundingBoxMax) * 0.5f;
        Vector3 size = boundingBoxMax - boundingBoxMin;
        float maxSize = Mathf.Max(size.x, size.y, size.z);
        
        // 计算合适的相机距离（平衡可见性和细节）
        float distance = maxSize * 5.0f; // 减少到5倍距离，更近距离观察
        
        // 计算边界框的对角线长度，确保相机在安全距离
        float diagonalLength = size.magnitude;
        distance = Mathf.Max(distance, diagonalLength * 2.0f); // 从3.0f减少到2.0f
        
        // 设置相机位置（从远处的等距离位置观察）
        // 使用更大的偏移，确保相机完全在模型外部
        Vector3 offset = new Vector3(3.0f, 2.0f, 3.0f).normalized * distance;
        Vector3 cameraPosition = center + offset;
        
        // 多重检查：确保相机不在边界框内部
        int attempts = 0;
        while (IsPointInsideBounds(cameraPosition, boundingBoxMin, boundingBoxMax) && attempts < 5)
        {
            distance *= 1.5f; // 每次增加50%距离
            offset = new Vector3(3.0f, 2.0f, 3.0f).normalized * distance;
            cameraPosition = center + offset;
            attempts++;
            Debug.LogWarning($"⚠️ 相机位置调整 #{attempts}，新距离: {distance:F1}");
        }
        
        // 最终安全检查：如果还在内部，使用极远距离
        if (IsPointInsideBounds(cameraPosition, boundingBoxMin, boundingBoxMax))
        {
            distance = diagonalLength * 10.0f; // 使用对角线长度的10倍
            offset = new Vector3(5.0f, 3.0f, 5.0f).normalized * distance;
            cameraPosition = center + offset;
            Debug.LogError($"🚨 强制设置相机到极远距离: {distance:F1}");
        }
        
        mainCamera.transform.position = cameraPosition;
        mainCamera.transform.LookAt(center);
        
        // 调整相机的视野范围，确保大型工件完全可见
        Camera cam = mainCamera.GetComponent<Camera>();
        if (cam != null)
        {
            // 使用适中的视野角度，平衡全貌和细节观察
            float recommendedFOV = 60f; // 使用60度视野，更适合近距离观察
            cam.fieldOfView = recommendedFOV;
            
            // 调整近裁剪面和远裁剪面
            cam.nearClipPlane = Mathf.Max(distance * 0.01f, 0.1f);
            cam.farClipPlane = Mathf.Max(distance * 5f, 20000f);
            
            Debug.Log($"📷 相机设置: FOV={recommendedFOV:F1}°, 近裁剪面={cam.nearClipPlane:F1}, 远裁剪面={cam.farClipPlane:F0}");
        }
        
        Debug.Log($"📷 相机已聚焦到点云中心: {center}, 距离: {distance:F1}");
        Debug.Log($"📷 相机位置: {cameraPosition}");
        Debug.Log($"📷 相机朝向: {(center - cameraPosition).normalized}");
        Debug.Log($"📷 工件尺寸: {size.x:F1} × {size.y:F1} × {size.z:F1}");
        
        // 检查轨道相机控制器并更新其参数
        OrbitCameraController orbitController = FindObjectOfType<OrbitCameraController>();
        if (orbitController != null)
        {
            // 创建边界框并聚焦
            Bounds bounds = new Bounds(center, size);
            orbitController.FocusOnBounds(bounds);
            
            Debug.Log($"📷 已更新轨道相机控制器，聚焦到边界框: {bounds}");
        }
    }
    
    /// <summary>
    /// 检查点是否在边界框内部
    /// </summary>
    private bool IsPointInsideBounds(Vector3 point, Vector3 min, Vector3 max)
    {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    /// <summary>
    /// LOD更新
    /// </summary>
    private void UpdateLOD()
    {
        if (mainCamera == null) return;
        
        float distance = Vector3.Distance(mainCamera.transform.position, transform.position);
        
        // 根据距离确定LOD级别
        int lodLevel = 0;
        for (int i = 0; i < lodDistances.Length; i++)
        {
            if (distance > lodDistances[i])
            {
                lodLevel = i + 1;
            }
        }
        
        // 这里可以实现LOD逻辑，比如隐藏远距离的网格或降低点密度
        // 简化实现：只是记录LOD级别
    }
    
    /// <summary>
    /// 清除点云
    /// </summary>
    public void ClearPointCloud()
    {
        Debug.Log($"🧹 开始清除点云，当前对象数量: {pointCloudMeshes.Count}");
        
        // 清除所有点云网格对象
        for (int i = pointCloudMeshes.Count - 1; i >= 0; i--)
        {
            GameObject meshObject = pointCloudMeshes[i];
            if (meshObject != null)
            {
                Debug.Log($"🗑️ 销毁对象: {meshObject.name}");
                
                // 如果是球体容器，先销毁所有子对象
                if (meshObject.name == "PointCloudSpheres")
                {
                    int childCount = meshObject.transform.childCount;
                    Debug.Log($"🗑️ 销毁球体容器的 {childCount} 个子对象");
                    
                    // 从后往前销毁子对象
                    for (int j = childCount - 1; j >= 0; j--)
                    {
                        Transform child = meshObject.transform.GetChild(j);
                        if (child != null)
                        {
                            DestroyImmediate(child.gameObject);
                        }
                    }
                }
                
                DestroyImmediate(meshObject);
            }
        }
        pointCloudMeshes.Clear();
        
        // 清除边界框
        if (boundingBoxObject != null)
        {
            Debug.Log("🗑️ 销毁边界框对象");
            DestroyImmediate(boundingBoxObject);
            boundingBoxObject = null;
        }
        
        // 清除调试标记
        GameObject debugMarker = GameObject.Find("DebugMarker_PointCloudCenter");
        if (debugMarker != null)
        {
            Debug.Log("🗑️ 销毁调试标记");
            DestroyImmediate(debugMarker);
        }
        
        // 清除所有子对象（确保完全清理）
        for (int i = transform.childCount - 1; i >= 0; i--)
        {
            Transform child = transform.GetChild(i);
            if (child != null)
            {
                Debug.Log($"🗑️ 清理遗留子对象: {child.name}");
                DestroyImmediate(child.gameObject);
            }
        }
        
        originalPoints = null;
        originalColors = null;
        
        Debug.Log("✅ 点云清除完成");
    }
    
    /// <summary>
    /// 创建默认点材质
    /// </summary>
    private Material CreateDefaultPointMaterial()
    {
        // 使用Standard着色器，确保在所有情况下都可见
        Material mat = new Material(Shader.Find("Standard"));
        mat.color = pointColor;
        
        // 设置为发光材质，确保可见性
        mat.SetColor("_EmissionColor", pointColor * 0.5f);
        mat.EnableKeyword("_EMISSION");
        
        // 设置渲染模式
        mat.SetFloat("_Mode", 0); // Opaque
        mat.SetFloat("_Metallic", 0);
        mat.SetFloat("_Glossiness", 0.5f);
        
        Debug.Log($"🎨 创建点云材质: {mat.shader.name}, 颜色: {pointColor}");
        return mat;
    }
    
    /// <summary>
    /// 创建线条材质
    /// </summary>
    private Material CreateLineMaterial(Color color)
    {
        Material mat = new Material(Shader.Find("Sprites/Default"));
        mat.color = color;
        return mat;
    }
    
    /// <summary>
    /// 使用优化的实例化渲染创建点云（高性能）
    /// </summary>
    private void CreatePointCloudSpheres(Vector3[] points, Color[] colors = null)
    {
        Debug.Log($"🔵 开始创建优化点云，点数: {points.Length}");
        
        // 计算工件尺寸，自动调整球体大小和显示点数
        Vector3 size = boundingBoxMax - boundingBoxMin;
        float maxDimension = Mathf.Max(size.x, size.y, size.z);
        
        // 根据工件大小自动调整球体尺寸
        float adaptivePointSize = Mathf.Clamp(maxDimension / 800f, 0.3f, 5f);
        Debug.Log($"🔧 自适应球体大小: {adaptivePointSize} (基于最大尺寸: {maxDimension})");
        
        // 智能采样：根据点云密度和性能需求调整显示点数
        int maxDisplayPoints = CalculateOptimalPointCount(points.Length, maxDimension);
        int step = Mathf.Max(1, points.Length / maxDisplayPoints);
        
        Debug.Log($"🔧 优化显示策略: 总点数={points.Length}, 显示点数={maxDisplayPoints}, 采样步长={step}");
        
        // 使用协程分批创建，避免卡顿
        StartCoroutine(CreatePointCloudCoroutine(points, colors, adaptivePointSize, step, maxDisplayPoints));
    }
    
    /// <summary>
    /// 计算最优点数显示策略（大幅优化大型工件）
    /// </summary>
    private int CalculateOptimalPointCount(int totalPoints, float maxDimension)
    {
        // 针对超大型工件（如82万点）的激进优化
        if (totalPoints > 500000) // 超大型点云（50万+点）
        {
            return Mathf.Clamp(totalPoints / 50, 1000, 2000); // 大幅降采样
        }
        else if (totalPoints > 100000) // 大型点云（10万+点）
        {
            return Mathf.Clamp(totalPoints / 20, 2000, 3000);
        }
        else if (maxDimension > 1000f) // 大型工件
        {
            return Mathf.Clamp(totalPoints / 8, 2000, 4000);
        }
        else if (maxDimension > 500f) // 中型工件
        {
            return Mathf.Clamp(totalPoints / 6, 3000, 6000);
        }
        else // 小型工件
        {
            return Mathf.Clamp(totalPoints / 4, 4000, 8000);
        }
    }
    
    /// <summary>
    /// 协程分批创建点云，避免卡顿
    /// </summary>
    private System.Collections.IEnumerator CreatePointCloudCoroutine(Vector3[] points, Color[] colors, float pointSize, int step, int maxPoints)
    {
        GameObject parentObject = new GameObject("PointCloudSpheres");
        parentObject.transform.SetParent(transform);
        
        // 创建优化的材质
        Material sharedMaterial = CreateOptimizedPointMaterial();
        
        int actualCount = 0;
        // 根据点数动态调整批大小
        int batchSize = maxPoints > 5000 ? 100 : 50; // 大型点云使用更大批次
        
        for (int i = 0; i < points.Length && actualCount < maxPoints; i += step)
        {
            // 创建点的可视化表示
            GameObject pointObj = CreateOptimizedPoint(points[i], pointSize, sharedMaterial);
            pointObj.name = $"Point_{actualCount}";
            pointObj.transform.SetParent(parentObject.transform);
            
            // 应用颜色（如果有）
            if (colors != null && i < colors.Length)
            {
                ApplyPointColor(pointObj, colors[i]);
            }
            
            actualCount++;
            
            // 每批处理后让出控制权，保持UI响应
            if (actualCount % batchSize == 0)
            {
                Debug.Log($"🔵 已创建 {actualCount}/{maxPoints} 个点...");
                yield return null; // 让出一帧
            }
        }
        
        pointCloudMeshes.Add(parentObject);
        Debug.Log($"🔵 优化点云创建完成: {actualCount} 个点，大小: {pointSize}");
        
        // 创建完成后进行最终优化
        OptimizePointCloudRendering(parentObject);
    }
    
    /// <summary>
    /// 创建优化的点材质
    /// </summary>
    private Material CreateOptimizedPointMaterial()
    {
        // 使用Unlit着色器提高性能
        Material mat = new Material(Shader.Find("Unlit/Color"));
        mat.color = pointColor;
        
        // 启用GPU实例化（如果支持）
        mat.enableInstancing = true;
        
        Debug.Log($"🎨 创建优化点云材质: {mat.shader.name}");
        return mat;
    }
    
    /// <summary>
    /// 创建优化的点对象
    /// </summary>
    private GameObject CreateOptimizedPoint(Vector3 position, float size, Material material)
    {
        // 使用简单的立方体代替球体，提高性能
        GameObject point = GameObject.CreatePrimitive(PrimitiveType.Cube);
        point.transform.position = position;
        point.transform.localScale = Vector3.one * size;
        
        // 移除碰撞器以提高性能
        Collider collider = point.GetComponent<Collider>();
        if (collider != null)
        {
            DestroyImmediate(collider);
        }
        
        // 应用材质
        Renderer renderer = point.GetComponent<Renderer>();
        renderer.material = material;
        renderer.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off; // 关闭阴影
        renderer.receiveShadows = false;
        
        return point;
    }
    
    /// <summary>
    /// 应用点颜色
    /// </summary>
    private void ApplyPointColor(GameObject pointObj, Color color)
    {
        Renderer renderer = pointObj.GetComponent<Renderer>();
        if (renderer != null && color != pointColor)
        {
            Material colorMaterial = new Material(renderer.material);
            colorMaterial.color = color;
            renderer.material = colorMaterial;
        }
    }
    
    /// <summary>
    /// 优化点云渲染性能
    /// </summary>
    private void OptimizePointCloudRendering(GameObject pointCloudParent)
    {
        // 启用静态批处理
        StaticBatchingUtility.Combine(pointCloudParent);
        
        // 设置LOD组（如果需要）
        LODGroup lodGroup = pointCloudParent.AddComponent<LODGroup>();
        LOD[] lods = new LOD[3];
        
        // 创建不同LOD级别的渲染器数组
        Renderer[] renderers = pointCloudParent.GetComponentsInChildren<Renderer>();
        
        lods[0] = new LOD(0.1f, renderers); // 近距离：显示所有点
        lods[1] = new LOD(0.05f, renderers.Take(renderers.Length / 2).ToArray()); // 中距离：显示一半点
        lods[2] = new LOD(0.01f, renderers.Take(renderers.Length / 4).ToArray()); // 远距离：显示四分之一点
        
        lodGroup.SetLODs(lods);
        lodGroup.RecalculateBounds();
        
        Debug.Log($"🚀 点云渲染优化完成，LOD级别: {lods.Length}");
    }
    
    /// <summary>
    /// 创建调试标记（临时）
    /// </summary>
    private void CreateDebugMarker()
    {
        // 在点云中心创建一个适中大小的红球
        Vector3 center = (boundingBoxMin + boundingBoxMax) * 0.5f;
        Vector3 size = boundingBoxMax - boundingBoxMin;
        float maxDimension = Mathf.Max(size.x, size.y, size.z);
        
        // 根据工件大小调整标记球体大小
        float markerSize = Mathf.Clamp(maxDimension / 20f, 10f, 100f);
        
        GameObject marker = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        marker.name = "DebugMarker_PointCloudCenter";
        marker.transform.position = center;
        marker.transform.localScale = Vector3.one * markerSize;
        
        Renderer markerRenderer = marker.GetComponent<Renderer>();
        markerRenderer.material = new Material(Shader.Find("Standard"));
        markerRenderer.material.color = Color.red;
        markerRenderer.material.SetColor("_EmissionColor", Color.red);
        markerRenderer.material.EnableKeyword("_EMISSION");
        
        marker.transform.SetParent(transform);
        
        Debug.Log($"🔴 创建调试标记球体在: {center}, 大小: {markerSize} (基于工件尺寸: {maxDimension:F1})");
    }
    
    /// <summary>
    /// 检查渲染状态
    /// </summary>
    private void CheckRenderingStatus()
    {
        Debug.Log($"🔍 检查渲染状态:");
        Debug.Log($"   - 点云网格数量: {pointCloudMeshes.Count}");
        
        for (int i = 0; i < pointCloudMeshes.Count; i++)
        {
            GameObject meshObj = pointCloudMeshes[i];
            if (meshObj != null)
            {
                Debug.Log($"   - 对象 {i}: {meshObj.name}, 激活={meshObj.activeInHierarchy}");
                
                // 检查是否是球体点云容器
                if (meshObj.name == "PointCloudSpheres")
                {
                    int childCount = meshObj.transform.childCount;
                    Debug.Log($"   - 球体点云容器，子对象数量: {childCount}");
                    
                    if (childCount > 0)
                    {
                        GameObject firstChild = meshObj.transform.GetChild(0).gameObject;
                        MeshRenderer renderer = firstChild.GetComponent<MeshRenderer>();
                        if (renderer != null)
                        {
                            Debug.Log($"   - 第一个球体: 激活={firstChild.activeInHierarchy}, 渲染器启用={renderer.enabled}");
                            Debug.Log($"   - 材质: {renderer.material.name}, 着色器: {renderer.material.shader.name}");
                        }
                    }
                }
                else
                {
                    // 原来的网格检查
                    MeshRenderer renderer = meshObj.GetComponent<MeshRenderer>();
                    MeshFilter filter = meshObj.GetComponent<MeshFilter>();
                    
                    if (renderer != null && filter != null)
                    {
                        Debug.Log($"   - 网格 {i}: 激活={meshObj.activeInHierarchy}, 渲染器启用={renderer.enabled}, 顶点数={filter.mesh.vertexCount}");
                        Debug.Log($"   - 材质: {renderer.material.name}, 着色器: {renderer.material.shader.name}");
                        Debug.Log($"   - 边界: {renderer.bounds}");
                    }
                }
            }
        }
    }
    
    /// <summary>
    /// 设置点云可见性
    /// </summary>
    public void SetVisible(bool visible)
    {
        gameObject.SetActive(visible);
    }
    
    /// <summary>
    /// 设置点云颜色
    /// </summary>
    public void SetPointColor(Color color)
    {
        pointColor = color;
        if (pointMaterial != null)
        {
            pointMaterial.color = color;
        }
    }
    
    /// <summary>
    /// 获取点云统计信息
    /// </summary>
    public string GetStatistics()
    {
        int totalPoints = originalPoints != null ? originalPoints.Length : 0;
        int meshCount = pointCloudMeshes.Count;
        
        return $"文件: {currentFileName}\n" +
               $"总点数: {totalPoints:N0}\n" +
               $"网格数: {meshCount}\n" +
               $"边界框: {boundingBoxMin} - {boundingBoxMax}";
    }
}