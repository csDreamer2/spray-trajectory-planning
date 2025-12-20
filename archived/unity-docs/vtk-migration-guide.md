# VTK迁移指南 - 从Unity到VTK

## 🔄 MainWindow.cpp 需要修改的部分

### 1. 头文件包含
```cpp
// 旧代码（Unity）
#include "UnityWidget.h"
#include "QtUnityBridge.h"

// 新代码（VTK）
#include "VTKWidget.h"
```

### 2. 成员变量
```cpp
// 旧代码
UnityWidget* m_unityView;
QtUnityBridge* m_unityBridge;

// 新代码
VTKWidget* m_vtkView;
```

### 3. setupUI() 函数
```cpp
// 旧代码
m_unityView = new UnityWidget(this);
m_unityView->setMinimumSize(800, 600);

QPushButton* initUnityBtn = new QPushButton("初始化Unity引擎", this);
connect(initUnityBtn, &QPushButton::clicked, m_unityView, &UnityWidget::InitializeUnity);

// 新代码
m_vtkView = new VTKWidget(this);
m_vtkView->setMinimumSize(800, 600);

// VTK不需要初始化按钮，直接就绪
```

### 4. 连接信号
```cpp
// 旧代码
connect(m_unityView, &UnityWidget::UnityReady, this, [this]() {
    m_statusLabel->setText("Unity 3D引擎已就绪");
});

// 新代码
connect(m_vtkView, &VTKWidget::ModelLoaded, this, [this](const QString& modelType, bool success) {
    if (success) {
        m_statusLabel->setText(QString("VTK: %1 加载成功").arg(modelType));
    }
});
```

### 5. 加载点云
```cpp
// 旧代码
void MainWindow::OnPointCloudLoadCompleted(bool success, const QJsonObject& pointCloudJson, const QString& errorMessage)
{
    if (success && m_unityView && m_unityBridge && m_unityBridge->IsConnected()) {
        m_unityView->ShowWorkpiece(pointCloudJson.toVariantMap());
    }
}

// 新代码
void MainWindow::OnPointCloudLoadCompleted(bool success, const QJsonObject& pointCloudJson, const QString& errorMessage)
{
    if (success && m_vtkView) {
        // 直接从文件路径加载
        QString filePath = pointCloudJson["file_path"].toString();
        m_vtkView->LoadPointCloud(filePath);
    }
}
```

### 6. 移除Unity相关代码
删除以下函数：
- `connectUnityBridgeSignals()`
- Unity Bridge相关的所有代码
- TCP通信相关代码

## 📝 完整的替换步骤

1. **备份当前代码**
2. **更新头文件** - MainWindow.h
3. **更新实现文件** - MainWindow.cpp
4. **测试编译**
5. **测试运行**

## ✅ 优势

- **更简单** - 不需要TCP通信
- **更直接** - 直接调用VTK API
- **更稳定** - 原生Qt集成
- **更快速** - 无进程间通信开销
