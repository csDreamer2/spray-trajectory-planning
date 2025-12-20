# Unity项目设置指南

## 🎯 目标
确保Unity项目正确配置，脚本正常工作，能够与Qt应用程序通信。

## 📋 Unity项目结构确认

### 当前项目结构
```
Unity/SpraySimulation/
├── Assets/
│   ├── Scenes/
│   │   └── SampleScene.unity          # 主场景
│   └── Scripts/
│       ├── QtCommunication.cs         # Qt通信脚本 ✅
│       ├── PointCloudRenderer.cs      # 点云渲染器 ✅
│       └── OrbitCameraController.cs   # 相机控制器 ✅
├── Library/                           # Unity缓存
├── ProjectSettings/                   # 项目设置
└── ...
```

## 🚀 Unity场景设置步骤

### 1. 打开Unity项目
1. 启动Unity Hub
2. 点击"Add"添加项目
3. 选择路径：`K:/vsCodeProjects/qtSpraySystem/Unity/SpraySimulation`
4. 打开项目

### 2. 检查脚本编译
1. 等待Unity完全加载
2. 查看Console窗口，确保没有编译错误
3. 如果有错误，检查脚本文件是否完整

### 3. 设置主场景
1. 打开`Assets/Scenes/SampleScene.unity`
2. 确保场景中有Main Camera

### 4. 添加Qt通信组件
1. 在Hierarchy中创建空GameObject：
   - 右键 → Create Empty
   - 命名为"QtCommunication"

2. 添加QtCommunication脚本：
   - 选中QtCommunication对象
   - 在Inspector中点击"Add Component"
   - 搜索"QtCommunication"
   - 添加脚本

3. 配置连接参数：
   - Qt Host: localhost
   - Qt Port: 12346

### 5. 添加点云渲染器（可选）
1. 创建空GameObject：
   - 右键 → Create Empty
   - 命名为"PointCloudRenderer"

2. 添加PointCloudRenderer脚本：
   - 选中对象
   - Add Component → PointCloudRenderer

### 6. 添加相机控制器（可选）
1. 选中Main Camera
2. Add Component → OrbitCameraController
3. 配置相机参数

## 🔧 脚本功能验证

### QtCommunication脚本
**功能**：
- 连接到Qt应用程序（端口12346）
- 接收点云数据
- 发送加载状态反馈
- 处理仿真命令

**验证方法**：
1. 点击Unity播放按钮▶️
2. 查看Console输出：
   ```
   Unity Qt通信模块启动
   尝试连接到Qt应用程序 localhost:12346
   ```
3. 如果Qt程序运行，应该看到：
   ```
   ✅ 成功连接到Qt应用程序
   ```

### PointCloudRenderer脚本
**功能**：
- 接收JSON格式的点云数据
- 渲染3D点云
- 自动调整相机视角
- 性能优化（LOD、批处理）

**验证方法**：
1. 在Qt程序中导入点云文件
2. Unity Console应该显示：
   ```
   🔧 处理工件数据...
   📊 接收到有效点云数据: sample_cube, 点数: 8000
   🔵 开始创建优化点云，点数: 8000
   ```

### OrbitCameraController脚本
**功能**：
- 鼠标控制相机旋转
- 滚轮缩放
- 自动聚焦到点云

**验证方法**：
1. 运行场景后用鼠标拖拽
2. 相机应该围绕中心旋转
3. 滚轮应该能缩放视图

## 🐛 常见问题解决

### 问题1：脚本编译错误
**症状**：Console显示红色错误信息

**解决方法**：
1. 检查脚本文件是否完整
2. 确认Unity版本兼容（推荐2022.3 LTS）
3. 重新导入脚本：
   - 右键Scripts文件夹 → Reimport

### 问题2：找不到脚本组件
**症状**：Add Component时搜索不到脚本

**解决方法**：
1. 等待脚本编译完成
2. 检查脚本文件名与类名是否一致
3. 重启Unity编辑器

### 问题3：连接Qt失败
**症状**：Console显示"连接Qt失败"

**解决方法**：
1. 确保Qt程序正在运行
2. 检查端口12346是否被占用
3. 检查防火墙设置
4. 验证IP地址（localhost/127.0.0.1）

### 问题4：点云不显示
**症状**：收到数据但看不到点云

**解决方法**：
1. 检查相机位置和方向
2. 调整点云大小参数
3. 检查材质和着色器
4. 查看Scene视图确认点云位置

## 📊 测试清单

### 基本功能测试
- [ ] Unity项目正常打开
- [ ] 脚本编译无错误
- [ ] QtCommunication组件已添加
- [ ] 场景可以正常播放

### 通信功能测试
- [ ] Unity连接到Qt程序
- [ ] 接收点云数据正常
- [ ] 发送反馈消息正常
- [ ] Console日志输出正确

### 渲染功能测试
- [ ] 点云正确显示
- [ ] 相机自动聚焦
- [ ] 鼠标控制正常
- [ ] 性能表现良好

## 🎮 Unity编辑器操作指南

### 基本操作
- **播放/停止**：点击▶️/⏹️按钮
- **暂停**：点击⏸️按钮
- **查看Console**：Window → General → Console
- **查看Inspector**：选中对象查看属性

### 调试技巧
1. **Console过滤**：
   - Clear：清除日志
   - Collapse：合并重复日志
   - Error/Warning/Info：过滤日志类型

2. **Scene视图**：
   - 鼠标中键：平移
   - 右键拖拽：旋转
   - 滚轮：缩放
   - F键：聚焦到选中对象

3. **Game视图**：
   - 显示运行时的实际效果
   - 可以测试鼠标交互

## 📚 相关资源

### Unity学习资源
- [Unity官方文档](https://docs.unity3d.com/)
- [Unity脚本API](https://docs.unity3d.com/ScriptReference/)
- [Unity教程](https://learn.unity.com/)

### 项目相关文档
- `docs/unity-window-embedding-guide.md` - 窗口嵌入指南
- `docs/unity-embedding-test-guide.md` - 测试指南
- `Unity/README.md` - Unity项目说明

---
**设置完成后**，你就可以：
1. 在Unity中点击▶️播放
2. 在Qt程序中导入点云文件
3. 观察Unity中的3D点云显示
4. 测试Qt-Unity窗口嵌入功能

**状态**: ✅ 脚本文件完整，可以开始设置