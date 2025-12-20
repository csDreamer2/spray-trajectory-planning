using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Unity点云渲染器
/// 负责接收Qt发送的点云数据并在Unity中进行可视化
/// </summary>
public class PointCloudRenderer : MonoBehaviour
{
    [Header("点云渲染设置")]
    public Material pointMaterial;
    public float pointSize = 0.01f;
    public Color pointColor = Color.white;
    public bool useVertexColors = false;
    
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
    }
    
    /// <summary>
    /// 加载点云数据
    /// </summary>
    public void LoadPointCloud(Vector3[] points, Color[] colors = null, Vector3 bboxMin = default, Vector3 bboxMax = default, string fileName = "")
    {
        Debug.Log($"🔧 开始加载点云: {fileName}, 点数: {points.Length}");
        
        // 清除之前的点云
        ClearPointCloud();
        
        // 保存原始数据
        originalPoints = points;
        originalColors = colors;
        boundingBoxMin = bboxMin;
        boundingBoxMax = bboxMax;
        currentFileName = fileName;
        
        // 创建点云网格
        CreatePointCloudMeshes(points, colors);
        
        // 创建边界框
        if (showBoundingBox && bboxMin != bboxMax)
        {
            CreateBoundingBox(bboxMin, bboxMax);
        }
        
        // 调整相机视角
        FocusCamera();
        
        Debug.Log($"✅ 点云加载完成: {fileName}");
    }
    
    /// <summary>
    /// 从JSON数据加载点云
    /// </summary>
    public void LoadPointCloudFromJson(string jsonData)
    {
        try
        {
            var data = JsonUtility.FromJson<PointCloudData>(jsonData);
            
            List<Vector3> points = new List<Vector3>();
            List<Color> colors = new List<Color>();
            
            // 解析点数据
            for (int i = 0; i < data.points.Length; i += 3)
            {
                if (i + 2 < data.points.Length)
                {
                    Vector3 point = new Vector3(data.points[i], data.points[i + 1], data.points[i + 2]);
                    points.Add(point);
                    
                    // 如果有颜色数据
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
            
            Vector3 bboxMin = new Vector3(data.boundingBoxMin[0], data.boundingBoxMin[1], data.boundingBoxMin[2]);
            Vector3 bboxMax = new Vector3(data.boundingBoxMax[0], data.boundingBoxMax[1], data.boundingBoxMax[2]);
            
            LoadPointCloud(points.ToArray(), colors.ToArray(), bboxMin, bboxMax, data.fileName);
        }
        catch (System.Exception e)
        {
            Debug.LogError($"❌ 点云JSON解析失败: {e.Message}");
        }
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
            
            mesh.vertices = vertices;
            mesh.colors = vertexColors;
            mesh.SetIndices(indices, MeshTopology.Points, 0);
            mesh.RecalculateBounds();
            
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
        
        // 计算合适的相机距离
        float distance = maxSize * 2.0f;
        
        // 设置相机位置（从斜上方观察）
        Vector3 cameraPosition = center + new Vector3(distance * 0.7f, distance * 0.5f, distance * 0.7f);
        
        mainCamera.transform.position = cameraPosition;
        mainCamera.transform.LookAt(center);
        
        Debug.Log($"📷 相机已聚焦到点云中心: {center}, 距离: {distance}");
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
        foreach (GameObject meshObject in pointCloudMeshes)
        {
            if (meshObject != null)
            {
                DestroyImmediate(meshObject);
            }
        }
        pointCloudMeshes.Clear();
        
        if (boundingBoxObject != null)
        {
            DestroyImmediate(boundingBoxObject);
            boundingBoxObject = null;
        }
        
        originalPoints = null;
        originalColors = null;
    }
    
    /// <summary>
    /// 创建默认点材质
    /// </summary>
    private Material CreateDefaultPointMaterial()
    {
        Material mat = new Material(Shader.Find("Sprites/Default"));
        mat.color = pointColor;
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