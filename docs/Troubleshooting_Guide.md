# 点云上传功能故障排除指南

## 常见错误及解决方案

### 1. "vector subscript out of range" 错误

**错误描述**: 运行时出现"Debug Assertion Failed"对话框，提示"vector subscript out of range"

**原因分析**: 
- 数组越界访问，通常发生在点云数据处理过程中
- `pointCount`与实际`points.size()`不一致
- 采样循环中索引超出范围

**解决方案**:
1. **已修复**: 在所有数组访问前添加边界检查
2. **已修复**: 确保`pointCount`与`points.size()`一致
3. **已修复**: 在采样循环中添加越界保护

**修复代码示例**:
```cpp
// 修复前（有问题）
for (int i = 0; i < pointCloudData.points.size(); i += sampleStep) {
    const QVector3D& point = pointCloudData.points[i]; // 可能越界
}

// 修复后（安全）
for (int i = 0; i < pointCloudData.points.size(); i += sampleStep) {
    if (i >= pointCloudData.points.size()) break; // 防止越界
    const QVector3D& point = pointCloudData.points[i];
}
```

### 2. PLY文件解析失败

**错误描述**: 提示"PLY文件解析失败"或"点云文件解析失败"

**可能原因**:
- PLY文件格式不正确
- 文件损坏或不完整
- PCL库版本不兼容
- 文件路径包含中文字符

**解决方案**:
1. **检查文件格式**: 确保PLY文件符合标准格式
2. **使用测试文件**: 先用提供的`test_data/sample_cube.ply`测试
3. **检查文件路径**: 避免中文路径和特殊字符
4. **验证文件完整性**: 用文本编辑器打开PLY文件检查内容

**标准PLY文件格式**:
```
ply
format ascii 1.0
comment Simple test cube
element vertex 8
property float x
property float y
property float z
end_header
-1.0 -1.0 -1.0
1.0 -1.0 -1.0
...
```

### 3. Unity连接失败

**错误描述**: "Unity连接失败，请检查Unity应用程序"

**可能原因**:
- Unity应用程序未启动
- TCP端口被占用或被防火墙阻止
- Unity脚本配置错误

**解决方案**:
1. **启动Unity应用程序**: 确保Unity项目正在运行
2. **检查端口**: 默认端口12346，确保未被占用
3. **防火墙设置**: 允许应用程序通过防火墙
4. **检查Unity脚本**: 确保`QtCommunication.cs`脚本已添加到场景

**Unity端检查清单**:
- [ ] Unity项目已打开并运行
- [ ] 场景中有GameObject包含`QtCommunication`脚本
- [ ] Console中显示"Unity Qt通信模块启动"
- [ ] 网络连接正常

### 4. 点云不显示

**错误描述**: Qt端显示加载成功，但Unity中看不到点云

**可能原因**:
- Unity相机位置不正确
- 点云渲染器未正确初始化
- 点云数据为空或无效
- 材质或Shader问题

**解决方案**:
1. **检查Unity Console**: 查看是否有错误信息
2. **验证数据接收**: 确认Unity收到点云数据
3. **调整相机**: 手动调整相机位置和角度
4. **检查渲染器**: 确保`PointCloudRenderer`脚本正常工作

**Unity调试步骤**:
```csharp
// 在Unity Console中查看这些日志
Debug.Log("🔧 开始加载点云: " + fileName);
Debug.Log("📊 接收到点云数据: " + pointCount + " 点");
Debug.Log("✅ 点云加载完成");
```

### 5. 性能问题

**错误描述**: 大型点云文件加载缓慢或程序卡死

**可能原因**:
- 点云文件过大
- 内存不足
- 采样率设置不当

**解决方案**:
1. **文件大小限制**: 默认限制500MB，可在代码中调整
2. **增加采样**: 减少发送到Unity的点数
3. **分批处理**: 大型点云分批加载
4. **内存优化**: 及时释放不用的资源

**性能优化设置**:
```cpp
// 调整最大点数限制
int sampleStep = qMax(1, pointCloudData.pointCount / 5000); // 减少到5000点

// 调整文件大小限制
parser->setMaxFileSize(200.0); // 限制为200MB
```

### 6. 编译错误

**错误描述**: 编译时出现PCL相关错误

**可能原因**:
- PCL库未正确安装
- CMake配置错误
- 头文件路径问题

**解决方案**:
1. **检查PCL安装**: 确保PCL 1.13.0已正确安装
2. **验证CMake配置**: 检查`CMAKE_PREFIX_PATH`设置
3. **重新配置**: 清除build目录重新配置

**重新编译步骤**:
```bash
# 清除旧的构建文件
Remove-Item -Recurse -Force build

# 重新配置
mkdir build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="K:\Kapps\Qt\6.10.1\msvc2022_64" -DBUILD_TESTS=OFF

# 编译
cmake --build build --config Debug
```

## 调试技巧

### 1. 启用详细日志

在Qt应用程序中启用详细日志输出：

```cpp
// 在main.cpp中添加
QLoggingCategory::setFilterRules("*.debug=true");
```

### 2. 使用调试器

1. 在Visual Studio中打开解决方案文件
2. 设置断点在关键函数
3. 使用F5启动调试
4. 逐步执行代码

### 3. Unity Console监控

在Unity中监控Console输出：
- 绿色信息：正常操作
- 黄色警告：需要注意的问题
- 红色错误：需要修复的问题

### 4. 网络连接测试

使用Python脚本测试TCP连接：

```python
import socket
import json

# 测试连接
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    sock.connect(('localhost', 12346))
    print("连接成功")
    
    # 发送测试消息
    message = {"type": "test", "data": "hello"}
    sock.send((json.dumps(message) + "\n").encode())
    
except Exception as e:
    print(f"连接失败: {e}")
finally:
    sock.close()
```

## 预防措施

### 1. 文件验证

在加载前验证PLY文件：
- 检查文件头格式
- 验证顶点数量
- 确认数据完整性

### 2. 内存管理

- 使用智能指针管理PCL对象
- 及时释放大型数据结构
- 监控内存使用情况

### 3. 错误处理

- 在所有关键操作中添加try-catch
- 提供用户友好的错误信息
- 实现优雅的错误恢复

### 4. 性能监控

- 记录处理时间
- 监控内存使用
- 设置合理的超时时间

## 联系支持

如果问题仍然存在，请提供以下信息：

1. **错误截图**: 包含完整的错误信息
2. **系统信息**: 操作系统版本、Qt版本、PCL版本
3. **测试文件**: 导致问题的PLY文件（如果可能）
4. **日志输出**: Qt和Unity的完整日志
5. **重现步骤**: 详细的操作步骤

**调试信息收集**:
```cpp
// 在出错位置添加
qDebug() << "Point count:" << pointCloudData.pointCount;
qDebug() << "Points size:" << pointCloudData.points.size();
qDebug() << "Sample step:" << sampleStep;
qDebug() << "Current index:" << i;
```

通过以上故障排除步骤，大部分问题都能得到解决。如果遇到新的问题，请参考日志输出进行分析。