# Unity项目设置指南

## 1. 创建Unity项目

### 步骤1: 创建新项目
1. 打开Unity Hub
2. 点击"New project"
3. 选择"3D (Built-in Render Pipeline)"模板
4. 项目名称: `SpraySimulation`
5. 位置: `Unity/SpraySimulation`
6. Unity版本: 推荐2022.3 LTS或更高版本
7. 点击"Create project"

### 步骤2: 项目结构
创建以下文件夹结构：
```
SpraySimulation/
├── Assets/
│   ├── Scripts/
│   │   ├── QtCommunication.cs
│   │   └── PointCloudRenderer.cs
│   ├── Materials/
│   ├── Scenes/
│   └── Prefabs/
└── ProjectSettings/
```

## 2. 添加脚本

### 方法1: 直接复制文件
1. 将`Unity/SpraySimulation/Assets/Scripts/`文件夹中的脚本复制到Unity项目的`Assets/Scripts/`文件夹
2. 在Unity编辑器中，右键点击`Assets`文件夹 → `Refresh`

### 方法2: 在Unity中创建脚本
1. 在Unity Project窗口中，右键点击`Assets/Scripts`文件夹
2. 选择`Create` → `C# Script`
3. 命名为`QtCommunication`
4. 双击打开脚本，复制粘贴代码内容
5. 保存文件
6. 重复步骤创建`PointCloudRenderer`脚本

## 3. 场景设置

### 步骤1: 创建GameObject
1. 在Hierarchy窗口中，右键点击空白区域
2. 选择`Create Empty`
3. 重命名为`QtCommunication`

### 步骤2: 添加脚本组件
1. 选中`QtCommunication` GameObject
2. 在Inspector窗口中，点击`Add Component`
3. 搜索并添加`QtCommunication`脚本
4. 再次点击`Add Component`
5. 搜索并添加`PointCloudRenderer`脚本

### 步骤3: 配置脚本参数
在Inspector窗口中配置以下参数：

**QtCommunication组件**:
- Qt Host: `localhost`
- Qt Port: `12346`

**PointCloudRenderer组件**:
- Point Size: `0.01`
- Point Color: `White`
- Show Bounding Box: `✓`
- Bounding Box Color: `Yellow`

## 4. 相机设置

### 步骤1: 配置主相机
1. 选中`Main Camera`
2. 在Inspector中设置：
   - Position: `(5, 3, 5)`
   - Rotation: `(15, -45, 0)`
   - Field of View: `60`
   - Clear Flags: 选择`纯色` (Solid Color)
   - Background: 选择黑色 `(0, 0, 0, 1)`

### 步骤2: 添加相机控制（可选）
1. 选中`Main Camera`
2. 在Inspector中点击`Add Component`
3. 搜索并添加`OrbitCameraController`脚本
4. 配置参数：
   - Distance: `10`
   - Min Distance: `2`
   - Max Distance: `50`
   - Rotation Speed: `2`
   - Zoom Speed: `2`

## 5. 材质设置（可选）

**注意**: `PointCloudRenderer`会自动创建默认材质，以下步骤是可选的。

### 如果要自定义材质：

#### 步骤1: 创建Materials文件夹
1. 在Project窗口中，右键点击`Assets`文件夹
2. 选择`Create` → `Folder`
3. 命名为`Materials`

#### 步骤2: 创建点云材质
1. 右键点击`Assets/Materials`文件夹
2. 选择`Create` → `Material`
3. 命名为`PointCloudMaterial`
4. 在Inspector中设置：
   - Shader: `Sprites/Default`
   - Main Color: `White`

#### 步骤3: 分配材质（可选）
1. 选中`QtCommunication` GameObject
2. 在`PointCloudRenderer`组件中
3. 将`PointCloudMaterial`拖拽到`Point Material`字段

**如果不设置自定义材质，系统会自动使用默认材质。**

## 6. 测试连接

### 步骤1: 运行Unity场景
1. 点击Unity编辑器顶部的`Play`按钮
2. 观察Console窗口的输出

### 步骤2: 检查连接状态
在Console中应该看到：
```
Unity Qt通信模块启动
尝试连接到Qt应用程序 localhost:12346
```

如果Qt应用程序未运行，会显示：
```
连接Qt失败: No connection could be made because the target machine actively refused it
5秒后重试连接...
```

## 7. 故障排除

### 问题1: 脚本无法添加为组件

**解决方案**:
1. 检查脚本文件是否在`Assets`文件夹内
2. 确保脚本文件名与类名一致
3. 检查脚本是否有编译错误
4. 在Unity中选择`Assets` → `Refresh`

### 问题2: 编译错误

**常见错误及解决方案**:

**错误**: `The type or namespace name 'System' could not be found`
**解决**: 确保脚本顶部有`using System;`

**错误**: `MonoBehaviour' does not contain a definition for 'StartCoroutine'`
**解决**: 确保类继承自`MonoBehaviour`

### 问题3: 连接失败

**检查清单**:
- [ ] Qt应用程序是否正在运行
- [ ] 端口12346是否被占用
- [ ] 防火墙是否阻止连接
- [ ] Unity Console是否显示连接尝试

## 8. 高级设置

### 性能优化
1. 在`PointCloudRenderer`中调整：
   - `Max Points Per Mesh`: 根据性能调整（默认65000）
   - `Enable LOD`: 启用距离优化
   - `LOD Distances`: 设置LOD切换距离

### 视觉效果
1. 创建自定义Shader用于点渲染
2. 添加后处理效果
3. 配置光照和阴影

### 调试工具
1. 启用Unity Profiler监控性能
2. 使用Frame Debugger分析渲染
3. 在Console中查看详细日志

## 9. 构建设置

### 开发构建
1. 选择`File` → `Build Settings`
2. 平台选择`PC, Mac & Linux Standalone`
3. 架构选择`x86_64`
4. 勾选`Development Build`
5. 点击`Build`

### 发布构建
1. 取消勾选`Development Build`
2. 设置`Compression Method`为`LZ4`
3. 优化`Stripping Level`
4. 点击`Build`

## 10. 完整测试流程

### 测试步骤
1. 启动Unity项目并点击Play
2. 启动Qt应用程序
3. 在Qt中点击"导入工件"
4. 选择PLY文件
5. 观察Unity中的点云显示

### 预期结果
- Unity Console显示连接成功
- Qt显示"点云数据已发送到Unity"
- Unity中显示点云和边界框
- 相机自动聚焦到点云中心

通过以上步骤，你应该能够成功设置Unity项目并与Qt应用程序进行通信。