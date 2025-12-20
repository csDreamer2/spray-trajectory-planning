# Unity独立应用程序构建指南

## 🎯 目标
构建Unity项目为独立的可执行文件，然后将其嵌入到Qt应用程序中，而不是嵌入Unity编辑器。

## 🔧 Unity构建步骤

### 1. 配置Unity构建设置

1. **打开Unity项目**：
   - 启动Unity Hub
   - 打开 `Unity/SpraySimulation` 项目

2. **打开构建设置**：
   - 菜单：File → Build Settings
   - 或快捷键：Ctrl+Shift+B

3. **配置平台设置**：
   - Platform: Windows
   - Architecture: x86_64
   - Target: Standalone

### 2. 配置Player设置

1. **点击"Player Settings"按钮**

2. **Resolution and Presentation设置**：
   ```
   Fullscreen Mode: Windowed
   Default Screen Width: 1280
   Default Screen Height: 720
   Resizable Window: ✓ (勾选)
   Run In Background: ✓ (勾选，重要！)
   ```

3. **Other Settings**：
   ```
   Company Name: SprayTech
   Product Name: SpraySimulation3D
   ```

4. **Publishing Settings**：
   ```
   Use Player Log: ✓ (勾选，便于调试)
   ```

### 3. 构建项目

1. **选择构建路径**：
   - 点击"Build"按钮
   - 选择输出目录：`build/bin/Debug/Unity/`
   - 文件名：`SpraySimulation3D.exe`

2. **等待构建完成**

3. **测试构建结果**：
   ```bash
   cd build/bin/Debug/Unity
   ./SpraySimulation3D.exe
   ```

## 🔧 修改Qt代码以启动独立Unity应用

### 1. 更新UnityWidget启动逻辑

修改 `InitializeUnity()` 方法来启动构建的Unity应用程序：

```cpp
bool UnityWidget::InitializeUnity()
{
    if (m_unityInitialized) {
        return true;
    }
    
    m_initButton->setText("正在启动Unity...");
    m_initButton->setEnabled(false);
    
    // 创建Unity进程
    m_unityProcess = new QProcess(this);
    
    // 连接信号
    connect(m_unityProcess, &QProcess::started, 
            this, &UnityWidget::OnUnityProcessStarted);
    connect(m_unityProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UnityWidget::OnUnityProcessFinished);
    connect(m_unityProcess, &QProcess::errorOccurred,
            this, &UnityWidget::OnUnityProcessError);
    
    // 查找构建的Unity应用程序
    QString unityAppPath = QApplication::applicationDirPath() + "/Unity/SpraySimulation3D.exe";
    
    if (!QFile::exists(unityAppPath)) {
        m_placeholderLabel->setText(
            "Unity 3D 仿真引擎\n\n"
            "❌ Unity应用程序未找到\n"
            "路径: " + unityAppPath + "\n\n"
            "请先构建Unity项目：\n"
            "1. 在Unity中打开SpraySimulation项目\n"
            "2. File → Build Settings\n"
            "3. 构建到 build/bin/Debug/Unity/ 目录"
        );
        m_initButton->setText("Unity应用未构建");
        emit UnityError("Unity应用程序未找到");
        return false;
    }
    
    // 启动Unity应用程序
    QStringList arguments;
    arguments << "-screen-width" << "1280";
    arguments << "-screen-height" << "720";
    arguments << "-screen-fullscreen" << "0"; // 窗口模式
    
    qDebug() << "启动Unity应用程序:" << unityAppPath;
    qDebug() << "启动参数:" << arguments;
    
    m_unityProcess->start(unityAppPath, arguments);
    
    return true;
}
```

### 2. 更新窗口查找逻辑

```cpp
HWND UnityWidget::findUnityWindow()
{
    // 查找构建的Unity应用程序窗口
    QStringList appWindowTitles = {
        "SpraySimulation3D",
        "SpraySimulation",
        "Unity Player"
    };
    
    HWND unityHwnd = nullptr;
    
    // 查找Unity Player窗口
    for (const QString& title : appWindowTitles) {
        unityHwnd = FindWindowA(nullptr, title.toLocal8Bit().constData());
        if (unityHwnd && IsWindowVisible(unityHwnd)) {
            qDebug() << "✅ 找到Unity应用程序窗口:" << title;
            break;
        }
    }
    
    // 验证窗口是否属于我们的Unity进程
    if (unityHwnd && m_unityProcess) {
        DWORD windowProcessId;
        GetWindowThreadProcessId(unityHwnd, &windowProcessId);
        
        if (windowProcessId == static_cast<DWORD>(m_unityProcess->processId())) {
            qDebug() << "✅ 窗口进程ID匹配";
            return unityHwnd;
        } else {
            qDebug() << "⚠️ 窗口进程ID不匹配";
        }
    }
    
    return unityHwnd;
}
```

## 🎯 优势

### 使用独立Unity应用程序的好处：

1. **纯净的3D视图**：
   - 只显示游戏内容，没有编辑器界面
   - 用户看到的就是最终的3D场景

2. **更好的性能**：
   - 没有编辑器开销
   - 专门为运行时优化

3. **更稳定的嵌入**：
   - Unity Player窗口更容易嵌入
   - 窗口属性更可预测

4. **部署友好**：
   - 可以独立分发
   - 不需要Unity编辑器

## 🔧 实施步骤

### 立即行动：

1. **构建Unity项目**：
   - 按照上述步骤构建SpraySimulation项目
   - 确保构建到正确的目录

2. **测试独立应用**：
   - 运行构建的exe文件
   - 确认3D内容正确显示
   - 验证TCP通信正常工作

3. **更新Qt代码**：
   - 修改启动逻辑指向构建的exe
   - 更新窗口查找逻辑
   - 测试窗口嵌入功能

### 预期结果：

- Qt界面中显示纯净的Unity 3D内容
- 没有Unity编辑器的干扰界面
- 更流畅的用户体验
- 更稳定的窗口嵌入

---

**这才是正确的Unity集成方案！** 让我们构建独立的Unity应用程序，而不是嵌入整个编辑器。