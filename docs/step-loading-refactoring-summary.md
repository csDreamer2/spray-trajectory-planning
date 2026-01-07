# STEP模型加载功能重构总结

## 重构目标

参考 `123/StepViewerWidget.cpp` 的简化实现，将当前复杂的异步STEP加载系统改为简单的同步加载方式。

## 已完成的工作

### 1. STEPModelTreeWidget 重构 ✅

**文件**: `src/UI/STEPModelTreeWidget.h` 和 `src/UI/STEPModelTreeWidget.cpp`

**主要变化**:
- ✅ 移除了所有异步加载相关代码（`STEPModelTreeWorker`, `QThread`等）
- ✅ 使用 `STEPCAFControl_Reader` 替代原来的 `STEPControl_Reader`
- ✅ 改为同步加载方式，直接在主线程中加载STEP文件
- ✅ 使用 `QTreeWidget` 替代 `QTreeView` + `QStandardItemModel`
- ✅ 简化了数据结构，使用 `QMap` 存储Actor和Shape
- ✅ 保留了基本的可见性控制功能

**新的API**:
```cpp
// 同步加载STEP文件
bool loadSTEPFile(const QString& filePath);

// 清空场景
void clearScene();

// 获取树形视图
QTreeWidget* getTreeWidget() const;

// 获取OCAF文档
Handle(TDocStd_Document) getDocument() const;
```

**信号**:
```cpp
// 加载完成信号
void loadCompleted(bool success, const QString& message);

// 部件可见性变化信号
void partVisibilityChanged(const QString& partName, bool visible);
```

### 2. VTKWidget 部分重构 ✅

**文件**: `src/UI/VTKWidget.h`

**主要变化**:
- ✅ 移除了 `LoadQuality` 枚举
- ✅ 移除了 `STEPLoaderWorker` 类声明
- ✅ 移除了异步加载相关的成员变量（`m_stepLoaderThread`, `m_stepLoaderWorker`, `m_loadingMutex`, `m_isLoading`）
- ✅ 移除了异步加载相关的槽函数声明
- ✅ 简化了 `LoadSTEPModel` 函数签名

**新的API**:
```cpp
// 简化的STEP加载接口
bool LoadSTEPModel(const QString& filePath, STEPModelTreeWidget* treeWidget = nullptr);
```

## 需要完成的工作

### 1. VTKWidget.cpp 清理 ⚠️

**文件**: `src/UI/VTKWidget.cpp`

**需要删除的函数**:
1. `void VTKWidget::LoadSTEPModelAsync(...)` - 第345行开始
2. `bool VTKWidget::LoadSTEPModelSync(...)` - 第478行开始
3. `vtkSmartPointer<vtkPolyData> VTKWidget::ConvertOCCTToVTK(...)` - 需要查找
4. `bool VTKWidget::CreateVTKActorFromPolyData(...)` - 需要查找
5. `void VTKWidget::CreateActorsForSTEPNode(...)` - 需要查找
6. `int VTKWidget::CountActors(...)` - 需要查找
7. `void VTKWidget::PrintNodesWithoutActors(...)` - 需要查找
8. `bool VTKWidget::LoadSTEPModelWithTree(...)` - 第2121行开始
9. `void VTKWidget::onSTEPLoaded(...)` - 第1663行开始
10. `void VTKWidget::onSTEPLoadFailed(...)` - 第1718行开始
11. `void VTKWidget::onSTEPLoadProgress(...)` - 第1733行开始
12. `void VTKWidget::onSTEPLoadProgressPercentage(...)` - 第1748行开始
13. `void VTKWidget::onTimeStatistics(...)` - 需要查找
14. `void STEPLoaderWorker::loadSTEPFile(...)` - 第1378行开始（在namespace外）

**需要修改的函数**:
1. `bool VTKWidget::LoadSTEPModel(...)` - 已部分修改，需要完善实现

**建议的实现方式**:
```cpp
bool VTKWidget::LoadSTEPModel(const QString& filePath, STEPModelTreeWidget* treeWidget)
{
    if (!treeWidget) {
        qWarning() << "VTKWidget: 需要提供STEPModelTreeWidget";
        return false;
    }
    
    // 连接可见性变化信号
    connect(treeWidget, &STEPModelTreeWidget::partVisibilityChanged,
            this, [this](const QString& partName, bool visible) {
                RefreshRender();
            }, Qt::UniqueConnection);
    
    // 加载STEP文件（同步）
    bool success = treeWidget->loadSTEPFile(filePath);
    
    if (success) {
        // 将STEPModelTreeWidget中的所有Actor添加到VTK渲染器
        // 注意：需要在STEPModelTreeWidget中添加公共方法来访问actorMap
        
        m_renderer->ResetCamera();
        m_renderWindow->Render();
        emit ModelLoaded("STEP", true);
    }
    
    return success;
}
```

### 2. STEPModelTreeWidget 增强 ⚠️

**需要添加的公共方法**:
```cpp
// 在 STEPModelTreeWidget.h 中添加
public:
    /**
     * @brief 将所有Actor添加到VTK渲染器
     * @param renderer VTK渲染器
     */
    void addActorsToRenderer(vtkRenderer* renderer);
    
    /**
     * @brief 从VTK渲染器移除所有Actor
     * @param renderer VTK渲染器
     */
    void removeActorsFromRenderer(vtkRenderer* renderer);
```

**实现**:
```cpp
// 在 STEPModelTreeWidget.cpp 中实现
void STEPModelTreeWidget::addActorsToRenderer(vtkRenderer* renderer)
{
    if (!renderer) return;
    
    for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
        renderer->AddActor(it.value());
    }
    
    qDebug() << "STEPModelTreeWidget: 添加了" << m_actorMap.size() << "个Actor到渲染器";
}

void STEPModelTreeWidget::removeActorsFromRenderer(vtkRenderer* renderer)
{
    if (!renderer) return;
    
    for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
        renderer->RemoveActor(it.value());
    }
    
    qDebug() << "STEPModelTreeWidget: 从渲染器移除了" << m_actorMap.size() << "个Actor";
}
```

### 3. MainWindow.cpp 更新 ⚠️

**文件**: `src/UI/MainWindow.cpp`

**需要修改的函数**: `void MainWindow::OnImportSTEPModel()`

**当前实现问题**:
- 使用了已废弃的异步加载方式
- 连接了不再存在的信号

**建议的新实现**:
```cpp
void MainWindow::OnImportSTEPModel()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择STEP模型文件",
        "data/model", "STEP文件 (*.step *.stp);;所有文件 (*.*)");
    
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            QMessageBox::warning(this, "文件错误", QString("文件不存在:\n%1").arg(fileName));
            return;
        }
        
        qDebug() << "开始加载STEP模型:" << fileName;
        
        if (m_statusPanel) {
            m_statusPanel->addLogMessage("INFO", QString("开始加载STEP文件: %1").arg(fileInfo.fileName()));
        }
        
        // 显示STEP模型树面板
        if (m_modelTreeDock) {
            m_modelTreeDock->show();
            m_modelTreeDock->raise();
        }
        
        // 同步加载STEP文件
        if (m_modelTreePanel && m_vtkView) {
            m_statusLabel->setText("正在加载STEP模型...");
            QApplication::processEvents();
            
            // 加载STEP文件（同步）
            bool success = m_modelTreePanel->loadSTEPFile(fileName);
            
            if (success) {
                // 将Actor添加到VTK渲染器
                m_modelTreePanel->addActorsToRenderer(m_vtkView->getRenderer());
                
                // 连接可见性变化信号
                connect(m_modelTreePanel, &STEPModelTreeWidget::partVisibilityChanged,
                        this, [this](const QString& partName, bool visible) {
                            if (m_vtkView) {
                                m_vtkView->RefreshRender();
                            }
                            if (m_statusPanel) {
                                m_statusPanel->addLogMessage("INFO", 
                                    QString("组件 %1: %2").arg(partName).arg(visible ? "显示" : "隐藏"));
                            }
                        }, Qt::UniqueConnection);
                
                // 重置相机
                m_vtkView->ResetCamera();
                
                m_statusLabel->setText("STEP模型加载成功");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("SUCCESS", "STEP模型加载完成");
                    m_statusPanel->addLogMessage("INFO", "可以在模型树中选择显示/隐藏零件");
                }
            } else {
                m_statusLabel->setText("STEP模型加载失败");
                if (m_statusPanel) {
                    m_statusPanel->addLogMessage("ERROR", "STEP模型加载失败");
                }
            }
        }
    }
}
```

### 4. VTKWidget 添加辅助方法 ⚠️

**需要在 VTKWidget.h 中添加**:
```cpp
public:
    /**
     * @brief 获取VTK渲染器
     */
    vtkRenderer* getRenderer() const { return m_renderer; }
    
    /**
     * @brief 刷新渲染
     */
    void RefreshRender();
```

**在 VTKWidget.cpp 中实现**:
```cpp
void VTKWidget::RefreshRender()
{
    if (m_renderWindow) {
        m_renderWindow->Render();
    }
    if (m_vtkWidget) {
        m_vtkWidget->update();
    }
}
```

### 5. 删除废弃的文件 ⚠️

以下文件可以删除（如果存在）:
- `src/Data/STEPModelTreeWorker.h`
- `src/Data/STEPModelTreeWorker.cpp`

## 编译和测试

### 编译步骤

1. 清理构建目录:
```bash
cd build
cmake --build . --target clean
```

2. 重新配置CMake:
```bash
cmake ..
```

3. 编译项目:
```bash
cmake --build . --config Debug
```

### 测试步骤

1. 启动程序
2. 点击"导入STEP模型"
3. 选择测试文件（如 `data/model/MPX3500.step`）
4. 观察：
   - 模型树是否正确显示
   - 3D视图是否正确渲染
   - 勾选/取消勾选是否能控制零件显示/隐藏

## 预期效果

### 优点
- ✅ 代码更简单，易于维护
- ✅ 没有线程同步问题
- ✅ 加载过程更直观
- ✅ 参考了成熟的实现（`123/StepViewerWidget`）

### 缺点
- ⚠️ 加载大型STEP文件时UI会阻塞
- ⚠️ 无法显示加载进度

### 改进建议
如果需要异步加载，建议使用 `QProgressDialog` 配合 `QTimer` 分步加载，而不是使用复杂的Worker线程。

## 参考文件

- `123/StepViewerWidget.h` - 参考实现的头文件
- `123/StepViewerWidget.cpp` - 参考实现的源文件
- `123/MainWindow.cpp` - 参考实现的主窗口

## 作者

王睿 (浙江大学)

## 日期

2026-01-07
