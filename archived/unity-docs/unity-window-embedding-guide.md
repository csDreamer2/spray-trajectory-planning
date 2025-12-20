# Unity窗口嵌入Qt指南

## 🎯 目标
将Unity 3D渲染窗口嵌入到Qt应用程序界面中，实现统一的用户体验。

## 🔧 实现方案
参考CSDN文章的方法，使用Windows API将Unity窗口嵌入到Qt Widget中。

### 核心技术
1. **FindWindow** - 查找Unity窗口句柄
2. **SetParent** - 将Unity窗口设置为Qt窗口的子窗口
3. **SetWindowLong** - 移除Unity窗口的标题栏和边框
4. **SetWindowPos** - 调整Unity窗口的位置和大小

## 📋 Unity项目构建步骤

### 方案1：构建独立应用程序（推荐）

1. **打开Unity项目**
   - 启动Unity Hub
   - 打开`Unity/SpraySimulation`项目

2. **配置构建设置**
   - 菜单：File → Build Settings
   - Platform: Windows
   - Architecture: x86_64
   - Target: Standalone

3. **Player设置**
   - 点击"Player Settings"
   - Resolution and Presentation:
     - Fullscreen Mode: Windowed
     - Default Screen Width: 1280
     - Default Screen Height: 720
     - Resizable Window: ✓
   - Other Settings:
     - Company Name: SprayTech
     - Product Name: SpraySimulation

4. **构建项目**
   - 点击"Build"
   - 选择输出目录：`build/bin/Debug/Unity/`
   - 等待构建完成

5. **测试构建**
   ```bash
   cd build/bin/Debug/Unity
   ./SpraySimulation.exe
   ```

### 方案2：使用Unity编辑器（当前方案）

当前实现使用Unity编辑器运行项目，优点是开发调试方便，缺点是窗口查找较复杂。

**窗口查找策略**：
- 通过窗口标题查找："Unity", "SpraySimulation - Unity"
- 通过窗口类名查找："UnityWndClass", "UnityContainerWndClass"
- 验证窗口进程ID匹配

## 🚀 使用步骤

### 1. 启动Qt应用程序
```bash
cd build/bin/Debug
./SprayTrajectoryPlanning.exe
```

### 2. 初始化Unity
- 点击"启动Unity应用程序"按钮
- 或手动启动Unity编辑器并打开项目

### 3. 运行Unity场景
- 在Unity编辑器中点击▶️播放按钮
- 等待Qt应用程序自动检测并嵌入Unity窗口

### 4. 验证嵌入
- Unity窗口应该出现在Qt界面的3D视图区域
- 占位标签自动隐藏
- 可以在Qt界面中直接操作Unity场景

## 🔍 窗口嵌入流程

```
1. Qt启动Unity进程
   ↓
2. 定时器每秒查找Unity窗口
   ↓
3. 找到Unity窗口句柄
   ↓
4. 使用SetParent嵌入窗口
   ↓
5. 移除标题栏和边框
   ↓
6. 调整窗口大小和位置
   ↓
7. 隐藏占位标签
   ↓
8. 完成嵌入
```

## 📊 关键代码

### 查找Unity窗口
```cpp
HWND UnityWidget::findUnityWindow()
{
    // 通过窗口标题查找
    HWND unityHwnd = FindWindowA(nullptr, "Unity");
    
    // 验证进程ID
    DWORD windowProcessId;
    GetWindowThreadProcessId(unityHwnd, &windowProcessId);
    
    return unityHwnd;
}
```

### 嵌入窗口
```cpp
void UnityWidget::embedWindowsWindow(HWND unityHwnd)
{
    // 设置为子窗口
    SetParent(unityHwnd, qtHwnd);
    
    // 移除边框
    LONG style = GetWindowLong(unityHwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME);
    SetWindowLong(unityHwnd, GWL_STYLE, style);
    
    // 调整大小
    SetWindowPos(unityHwnd, HWND_TOP, x, y, width, height, SWP_SHOWWINDOW);
}
```

### 窗口大小调整
```cpp
void UnityWidget::resizeEvent(QResizeEvent* event)
{
    if (m_unityWindowHandle) {
        SetWindowPos(m_unityWindowHandle, HWND_TOP,
                     x, y, width, height, SWP_NOZORDER);
    }
}
```

## ⚠️ 注意事项

### Windows平台限制
- 窗口嵌入仅支持Windows平台
- 需要Windows API支持
- 跨平台需要不同实现

### Unity窗口查找
- Unity编辑器窗口标题可能变化
- 需要多种查找策略
- 验证进程ID确保正确性

### 窗口交互
- 鼠标事件自动转发到Unity
- 键盘事件需要焦点管理
- 窗口大小调整需要同步

## 🐛 故障排除

### 问题1：找不到Unity窗口
**症状**：定时器一直运行，无法嵌入窗口

**解决方法**：
1. 确认Unity已启动并运行场景
2. 检查Unity窗口标题是否匹配
3. 查看Debug输出的窗口信息
4. 尝试手动查找窗口句柄

### 问题2：窗口嵌入后无法交互
**症状**：Unity窗口显示但无法操作

**解决方法**：
1. 检查窗口样式设置
2. 确认窗口Z-order正确
3. 验证鼠标事件转发
4. 检查窗口焦点状态

### 问题3：窗口大小不正确
**症状**：Unity窗口大小与Qt区域不匹配

**解决方法**：
1. 检查SetWindowPos参数
2. 确认坐标系转换正确
3. 处理DPI缩放问题
4. 监听Qt窗口大小变化

## 📈 性能优化

### 窗口查找优化
- 减少查找频率（1秒→2秒）
- 找到后立即停止定时器
- 缓存窗口句柄

### 渲染性能
- Unity使用独立渲染线程
- 不影响Qt主线程
- 保持60FPS流畅度

### 内存管理
- 及时释放窗口资源
- 清理定时器
- 处理进程退出

## 🔮 未来改进

### 短期改进
1. **自动构建Unity项目** - 集成到CMake构建流程
2. **更智能的窗口查找** - 使用EnumWindows遍历所有窗口
3. **窗口状态保存** - 记住窗口位置和大小

### 长期规划
1. **跨平台支持** - Linux和macOS的窗口嵌入
2. **多Unity实例** - 支持多个Unity窗口
3. **WebGL方案** - 使用QWebEngineView显示Unity WebGL

## 📚 参考资料

- [CSDN文章：Qt嵌入Unity窗口](https://blog.csdn.net/HXX904/article/details/134827114)
- [Windows API文档](https://docs.microsoft.com/en-us/windows/win32/api/)
- [Qt窗口嵌入文档](https://doc.qt.io/qt-6/qwindow.html)

---
**实现日期**: 2024-12-18  
**状态**: ✅ 已实现  
**平台**: Windows Only