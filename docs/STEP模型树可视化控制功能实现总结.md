# STEP模型树可视化控制功能实现总结

**作者**: AI Assistant  
**日期**: 2025-12-25  
**版本**: v1.0

## 功能概述

实现了STEP模型的树状结构显示和可视化控制功能，用户可以：
1. 导入STEP文件并解析为树状结构
2. 在树形控件中查看所有装配体和零件
3. 通过勾选/取消勾选控制各个零件的显示/隐藏
4. 支持父子节点联动（勾选装配体会影响所有子零件）

## 技术架构

### 三层绑定模型

```
[QTreeWidgetItem (UI层)]
         ↓
    (quintptr指针)
         ↓
[STEPTreeNode (数据层)]
         ↓
    TopoDS_Shape (几何)
         ↓
[vtkActor (渲染层)]
```

## 核心修改

### 1. 数据结构增强 (`src/Data/STEPModelTree.h`)

```cpp
struct STEPTreeNode {
    QString name;
    TopoDS_Shape shape;
    bool isVisible;
    bool isAssembly;
    int level;
    std::vector<std::shared_ptr<STEPTreeNode>> children;
    vtkSmartPointer<vtkActor> actor;  // 新增：VTK Actor
    // ...
};
```

**关键点**：
- 每个节点都有自己的`vtkActor`用于独立控制显示
- 使用`vtkSmartPointer`自动管理内存

### 2. UI控件增强 (`src/UI/STEPModelTreeWidget.h/cpp`)

#### 新增信号
```cpp
signals:
    void nodeVisibilityToggled(std::shared_ptr<STEPTreeNode> node, bool visible);
```

#### 新增槽函数
```cpp
private slots:
    void onItemChanged(QStandardItem* item);  // 监听复选框变化
```

#### 核心实现

**节点指针存储**：
```cpp
// 使用quintptr安全存储指针
quintptr nodePtr = reinterpret_cast<quintptr>(node.get());
nameItem->setData(QVariant::fromValue(nodePtr), Qt::UserRole);
```

**节点指针恢复**：
```cpp
quintptr nodePtr = data.value<quintptr>();
STEPTreeNode* rawPtr = reinterpret_cast<STEPTreeNode*>(nodePtr);
auto sharedNode = findNodeInTree(rootNode, rawPtr);
```

**可见性控制**：
```cpp
void STEPModelTreeWidget::onItemChanged(QStandardItem* item)
{
    auto node = getNodeFromItem(item);
    bool visible = (item->checkState() == Qt::Checked);
    
    // 更新节点状态
    node->isVisible = visible;
    
    // 更新Actor可见性
    if (node->actor) {
        node->actor->SetVisibility(visible ? 1 : 0);
    }
    
    // 递归更新子节点
    setChildrenVisibility(item, visible);
    
    // 通知VTK刷新
    emit nodeVisibilityToggled(node, visible);
}
```

### 3. VTK可视化增强 (`src/UI/VTKWidget.h/cpp`)

#### 新增方法

```cpp
// 为STEP模型树创建Actors
bool LoadSTEPModelWithTree(const QString& filePath, 
                          std::shared_ptr<STEPTreeNode> rootNode, 
                          LoadQuality quality = LoadQuality::Balanced);

// 递归为节点创建Actor
void CreateActorsForSTEPNode(std::shared_ptr<STEPTreeNode> node);

// 刷新渲染
void RefreshRender();
```

#### 核心实现

**为每个零件创建独立Actor**：
```cpp
void VTKWidget::CreateActorsForSTEPNode(std::shared_ptr<STEPTreeNode> node)
{
    if (!node || node->shape.IsNull()) return;
    
    // 1. 转换Shape为VTK PolyData
    vtkSmartPointer<vtkPolyData> polyData = ConvertOCCTToVTK(node->shape);
    
    // 2. 创建Mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    
    // 3. 创建Actor并绑定到节点
    node->actor = vtkSmartPointer<vtkActor>::New();
    node->actor->SetMapper(mapper);
    
    // 4. 设置显示属性
    if (node->isAssembly) {
        node->actor->GetProperty()->SetColor(0.7, 0.7, 0.8);
        node->actor->GetProperty()->SetOpacity(0.5);
    } else {
        // 根据层级设置不同颜色
        double r = 0.3 + (node->level % 3) * 0.2;
        double g = 0.4 + (node->level % 4) * 0.15;
        double b = 0.5 + (node->level % 5) * 0.1;
        node->actor->GetProperty()->SetColor(r, g, b);
    }
    
    // 5. 设置初始可见性
    node->actor->SetVisibility(node->isVisible ? 1 : 0);
    
    // 6. 添加到渲染器
    m_renderer->AddActor(node->actor);
    
    // 7. 递归处理子节点
    for (auto& child : node->children) {
        CreateActorsForSTEPNode(child);
    }
}
```

### 4. 主窗口集成 (`src/UI/MainWindow.cpp`)

#### 连接信号槽

```cpp
void MainWindow::connectModelTreeToVTK()
{
    // 连接节点可见性变化信号到VTK刷新
    connect(m_modelTreePanel, &STEPModelTreeWidget::nodeVisibilityToggled,
            this, [this](std::shared_ptr<STEPTreeNode> node, bool visible) {
                // 刷新VTK渲染
                if (m_vtkView) {
                    m_vtkView->RefreshRender();
                }
            });
}
```

#### 加载流程

```cpp
void MainWindow::OnImportSTEPModel()
{
    // 1. 选择文件
    QString fileName = QFileDialog::getOpenFileName(...);
    
    // 2. 加载STEP模型树
    m_modelTreePanel->loadSTEPFile(fileName);
    
    // 3. 加载完成后创建VTK可视化
    connect(m_modelTreePanel, &STEPModelTreeWidget::loadCompleted,
            this, [this, fileName](bool success, const QString& message) {
                if (success) {
                    // 获取根节点
                    auto rootNode = m_modelTreePanel->getModelTree()->getRootNode();
                    
                    // 创建VTK Actors
                    m_vtkView->LoadSTEPModelWithTree(fileName, rootNode);
                    
                    // 连接信号
                    connectModelTreeToVTK();
                }
            });
}
```

## 关键技术点

### 1. 内存安全

**问题**：Qt不支持直接存储`shared_ptr`到`QVariant`  
**解决**：使用`quintptr`作为中间类型

```cpp
// 存储
quintptr ptr = reinterpret_cast<quintptr>(node.get());
item->setData(QVariant::fromValue(ptr), Qt::UserRole);

// 恢复
quintptr ptr = data.value<quintptr>();
STEPTreeNode* rawPtr = reinterpret_cast<STEPTreeNode*>(ptr);
```

### 2. 异步安全

**问题**：Lambda捕获的`shared_ptr`可能在异步调用时失效  
**解决**：显式复制`shared_ptr`

```cpp
auto rootNodeCopy = rootNode;  // 复制shared_ptr
QTimer::singleShot(100, this, [this, rootNodeCopy]() {
    // 使用rootNodeCopy，确保对象有效
    m_vtkView->LoadSTEPModelWithTree(fileName, rootNodeCopy);
});
```

### 3. 异常处理

添加了多层异常保护：
- OpenCASCADE异常 (`Standard_Failure`)
- C++标准异常 (`std::exception`)
- 未知异常 (`...`)

```cpp
try {
    CreateActorsForSTEPNode(rootNode);
} catch (const Standard_Failure& e) {
    qCritical() << "OpenCASCADE异常:" << e.GetMessageString();
} catch (const std::exception& e) {
    qCritical() << "标准异常:" << e.what();
} catch (...) {
    qCritical() << "未知异常";
}
```

### 4. 父子节点联动

```cpp
void STEPModelTreeWidget::setChildrenVisibility(QStandardItem* item, bool visible)
{
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* child = item->child(i, 0);
        
        // 暂时断开信号避免递归触发
        disconnect(m_modelTree->getQtModel(), &QStandardItemModel::itemChanged,
                   this, &STEPModelTreeWidget::onItemChanged);
        
        // 设置子项状态
        child->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
        
        // 重新连接信号
        connect(m_modelTree->getQtModel(), &QStandardItemModel::itemChanged,
                this, &STEPModelTreeWidget::onItemChanged);
        
        // 更新Actor
        auto childNode = getNodeFromItem(child);
        if (childNode && childNode->actor) {
            childNode->actor->SetVisibility(visible ? 1 : 0);
        }
        
        // 递归处理
        setChildrenVisibility(child, visible);
    }
}
```

## 使用方法

### 1. 导入STEP模型

1. 点击菜单 "文件 -> 导入STEP模型" 或按 `Ctrl+S`
2. 选择STEP文件（.step或.stp）
3. 等待模型树构建完成
4. 自动创建3D可视化

### 2. 控制零件显示

1. 在右侧"STEP模型树"面板中查看树状结构
2. 勾选/取消勾选复选框控制零件显示
3. 勾选装配体会同时控制所有子零件
4. 3D视图会实时更新

### 3. 查看模型

- 使用鼠标左键拖动旋转视图
- 使用鼠标滚轮缩放
- 点击"重置视图"按钮恢复默认视角
- 点击"适应场景"按钮自动调整视角

## 调试信息

程序会输出详细的调试信息：

```
MainWindow: STEP模型树加载成功，准备创建VTK可视化
MainWindow: 获取根节点: 成功
MainWindow: 延迟100ms后创建VTK可视化
MainWindow: 开始创建VTK可视化...
VTKWidget: 开始为STEP模型树创建Actors
VTKWidget: 根节点名称: MPX3500
VTKWidget: 根节点子节点数: 8
VTKWidget: 为节点创建Actor: MPX3500 Base 层级: 2
VTKWidget: PolyData转换成功，点数: 12345
VTKWidget: Actor创建并添加成功: MPX3500 Base
...
MainWindow: VTK可视化创建成功
```

## 已知问题和限制

1. **性能**：大型装配体（>100个零件）可能需要较长时间创建Actors
2. **内存**：每个零件都有独立的Actor，内存占用较大
3. **颜色**：当前使用简单的层级颜色方案，未来可以支持自定义颜色

## 未来改进方向

1. **性能优化**
   - 使用LOD（Level of Detail）技术
   - 延迟创建Actor（仅创建可见零件的Actor）
   - 使用实例化渲染减少内存占用

2. **功能增强**
   - 支持零件高亮显示
   - 支持零件选择和属性查看
   - 支持零件颜色自定义
   - 支持零件透明度调整

3. **用户体验**
   - 添加搜索功能
   - 添加过滤功能（按类型、名称等）
   - 添加右键菜单（隐藏、仅显示此项等）
   - 添加批量操作（全部显示、全部隐藏等）

## 测试建议

1. **小型模型测试**（<10个零件）
   - 验证基本功能
   - 检查可见性控制

2. **中型模型测试**（10-50个零件）
   - 验证性能
   - 检查父子节点联动

3. **大型模型测试**（>50个零件）
   - 压力测试
   - 内存使用监控

## 参考文档

- [OpenCASCADE集成指南](opencascade-integration-guide.md)
- [VTK集成指南](vtk-integration-guide.md)
- [STEP模型树功能说明](STEP模型树功能说明.md)
