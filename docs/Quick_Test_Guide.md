# 快速测试指南

## Unity设置步骤

### 1. 创建Unity项目
1. 打开Unity Hub
2. 点击"New project"
3. 选择"3D (Built-in Render Pipeline)"模板
4. 项目命名为`SpraySimulation`
5. 项目位置选择`Unity/SpraySimulation`
6. 点击"Create project"

### 2. 添加脚本
1. 在Project窗口中，右键点击`Assets`文件夹
2. 选择`Create` → `Folder`，命名为`Scripts`
3. 将以下两个脚本文件复制到`Assets/Scripts/`文件夹：
   - `Unity/SpraySimulation/Assets/Scripts/QtCommunication.cs`
   - `Unity/SpraySimulation/Assets/Scripts/PointCloudRenderer.cs`
4. 在Unity中右键点击`Assets` → `Refresh`

### 3. 设置场景
1. 在Hierarchy窗口中，右键点击空白区域
2. 选择`Create Empty`
3. 重命名为`QtCommunication`
4. 选中这个GameObject，在Inspector中点击`Add Component`
5. 搜索并添加`QtCommunication`脚本
6. 再次点击`Add Component`，添加`PointCloudRenderer`脚本

### 4. 配置参数
在Inspector中确认以下设置：
- **QtCommunication**:
  - Qt Host: `localhost`
  - Qt Port: `12346`
- **PointCloudRenderer**:
  - Point Size: `0.01`
  - Point Color: `White`
  - Show Bounding Box: `✓`

### 5. 配置相机
选中`Main Camera`，在Inspector中设置：
- Clear Flags: 选择`纯色` (Solid Color)
- Background: 选择黑色 `(0, 0, 0, 1)`
- Field of View: `60`

### 6. 运行Unity
点击Unity编辑器顶部的`Play`按钮

## Qt应用程序测试

### 1. 启动程序
运行编译好的Qt应用程序：
```
build\bin\Debug\SprayTrajectoryPlanning.exe
```

### 2. 测试点云上传
1. 点击菜单"文件" → "导入工件"
2. 选择测试文件`test_data/sample_cube.ply`
3. 等待解析完成

### 3. 观察结果
- Qt应用程序应显示解析成功信息
- Unity Console应显示连接和数据接收信息
- Unity Scene视图中应显示点云（8个点的立方体）

## 预期输出

### Qt Console输出
```
点云解析成功 - 文件: sample_cube.ply 点数: 8 实际大小: 8
开始采样 - 总点数: 8 采样步长: 1
采样完成 - 发送点数: 8
```

### Unity Console输出
```
Unity Qt通信模块启动
尝试连接到Qt应用程序 localhost:12346
✅ 成功连接到Qt应用程序
📥 收到Qt消息: workpiece_data
🔧 处理工件数据...
📊 接收到点云数据: sample_cube.ply, 点数: 8
🔧 开始加载点云: sample_cube.ply, 点数: 8
📊 创建了 1 个点云网格，总点数: 8
📷 相机已聚焦到点云中心: (0.0, 0.0, 0.0), 距离: 4.0
✅ 点云加载完成: sample_cube.ply
✅ 点云加载完成: sample_cube.ply
```

## 故障排除

### 问题1: Unity脚本无法添加
**解决方案**:
1. 确保脚本文件在`Assets/Scripts/`文件夹内
2. 检查脚本是否有编译错误（Console窗口）
3. 右键点击`Assets` → `Refresh`

### 问题2: Qt程序崩溃
**解决方案**:
1. 使用提供的测试文件`test_data/sample_cube.ply`
2. 检查文件路径是否包含中文字符
3. 确保PLY文件格式正确

### 问题3: Unity连接失败
**解决方案**:
1. 确保Qt程序先启动
2. 检查防火墙设置
3. 确认端口12346未被占用

### 问题4: 点云不显示
**解决方案**:
1. 检查Unity Console是否有错误
2. 在Scene视图中查找点云（可能很小）
3. 调整相机位置和角度

## 调试技巧

### 启用详细日志
在Qt的main.cpp中添加：
```cpp
QLoggingCategory::setFilterRules("*.debug=true");
```

### Unity调试
在Unity Console中查看所有日志输出，特别注意：
- 连接状态信息
- 数据接收确认
- 错误和警告信息

### 网络测试
如果连接有问题，可以使用telnet测试：
```cmd
telnet localhost 12346
```

## 成功标志

测试成功的标志：
1. ✅ Unity成功连接到Qt
2. ✅ Qt成功解析PLY文件
3. ✅ 数据成功传输到Unity
4. ✅ Unity中显示点云和边界框
5. ✅ 相机自动聚焦到点云

如果所有步骤都成功，说明基本功能正常，可以尝试加载更复杂的PLY文件。