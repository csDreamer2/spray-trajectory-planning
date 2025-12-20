# STEP模型树功能说明

## 🎯 功能概述

STEP模型树功能允许用户像CAD软件一样查看和管理复杂STEP文件的层次结构，实现对单个组件的精确控制。

### 🔍 主要特性

- **📊 层次结构解析** - 完整解析STEP文件的装配体结构
- **🎛️ 选择性显示** - 可以单独显示/隐藏任意组件
- **🔍 智能搜索** - 按名称快速查找组件
- **📋 详细信息** - 显示组件类型、标签等详细信息
- **🎨 可视化控制** - 透明度、颜色等显示属性控制

## 🏗️ 架构设计

### 核心类结构

```cpp
STEPModelTree           // 核心模型树管理器
├── STEPTreeNode       // 树节点数据结构
├── QStandardItemModel // Qt模型适配
└── OpenCASCADE XCAF   // STEP文件解析

STEPModelTreeWidget    // 完整的树形控件
├── QTreeView         // 树形视图
├── 上下文菜单        // 右键操作
└── 信号连接          // 事件处理

ModelTreeDockWidget    // 停靠窗口集成
├── 控制面板          // 操作按钮
├── 显示选项          // 显示设置
└── 状态信息          // 统计数据
```

### 数据流程

```
STEP文件 → XCAF解析 → 树结构构建 → Qt模型 → 用户界面
    ↓           ↓           ↓         ↓         ↓
  几何数据   → 标签信息   → 节点对象  → 树形项  → 可视化
```

## 🚀 使用方法

### 1. 基本集成

```cpp
#include "UI/ModelTreeDockWidget.h"

// 在主窗口中创建模型树停靠窗口
ModelTreeDockWidget* modelTreeDock = new ModelTreeDockWidget(this);
addDockWidget(Qt::LeftDockWidgetArea, modelTreeDock);

// 连接信号
connect(modelTreeDock, &ModelTreeDockWidget::modelVisibilityChanged,
        this, &MainWindow::onModelVisibilityChanged);
connect(modelTreeDock, &ModelTreeDockWidget::modelSelectionChanged,
        this, &MainWindow::onModelSelectionChanged);
```

### 2. 加载STEP文件

```cpp
void MainWindow::loadSTEPFile(const QString& filePath) {
    // 加载到模型树
    modelTreeDock->loadSTEPFile(filePath);
}

void MainWindow::onModelVisibilityChanged(const std::vector<TopoDS_Shape>& visibleShapes) {
    // 更新3D渲染器显示
    vtkRenderer->RemoveAllActors();
    
    for (const auto& shape : visibleShapes) {
        auto actor = createActorFromShape(shape);
        vtkRenderer->AddActor(actor);
    }
    
    vtkRenderWindow->Render();
}
```

### 3. 高级功能

```cpp
// 查找特定组件
auto nodes = modelTree->findNodesByName("MPX3500");
for (auto& node : nodes) {
    modelTree->setNodeVisibility(node, true);
}

// 获取模型统计
auto stats = modelTree->getModelStats();
qDebug() << "Total nodes:" << stats.totalNodes;
qDebug() << "Assemblies:" << stats.assemblies;
qDebug() << "Parts:" << stats.parts;

// 设置透明度
// 在渲染时应用透明度设置
```

## 🎨 用户界面

### 模型树视图

```
📁 MPX3500 L-Type                    [✓] 装配体  显示  0:1:1:1
├── 📁 MPX3500 B0 Envelope L-Type   [✓] 装配体  显示  0:1:1:2
│   ├── 🔧 Revolve7                 [✓] 零件    显示  0:1:1:3
│   ├── 🔧 Revolve12                [✓] 零件    显示  0:1:1:4
│   └── 🔧 Revolve13                [✓] 零件    显示  0:1:1:5
├── 📁 MPX3500 BASE L-Type          [✓] 装配体  显示  0:1:1:6
│   ├── 🔧 HW14002270_2             [✓] 零件    显示  0:1:1:7
│   └── 🔧 HW13721356_ASM           [✓] 零件    显示  0:1:1:8
└── 📁 MPX3500 F axis               [✓] 装配体  显示  0:1:1:9
    ├── 🔧 HW03002370               [✓] 零件    显示  0:1:1:10
    └── 🔧 HW03002373               [✓] 零件    显示  0:1:1:11
```

### 控制面板

```
┌─ 控制面板 ─────────────────┐
│ [全显] [全隐] [展开] [折叠] [刷新] │
└─────────────────────────┘

┌─ 显示选项 ─────────────────┐
│ ☑ 显示装配体                │
│ ☑ 显示零件                  │
│ 透明度: ████░░░░░░ 40%      │
└─────────────────────────┘

┌─ 状态信息 ─────────────────┐
│ 总节点: 156                │
│ 可见节点: 89               │
│ 选中节点: 3                │
│ ████████████████ 100%      │
│ 加载完成                   │
└─────────────────────────┘
```

## 🔧 技术实现

### STEP文件解析

```cpp
// 使用OpenCASCADE XCAF框架
STEPCAFControl_Reader reader;
Handle(TDocStd_Document) doc;
reader.SetDocument(doc);
reader.ReadFile(filePath);
reader.Transfer(doc);

// 获取XCAF工具
Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
Handle(XCAFDoc_ColorTool) colorTool = XCAFDoc_DocumentTool::ColorTool(doc->Main());

// 解析层次结构
TDF_LabelSequence freeShapes;
shapeTool->GetFreeShapes(freeShapes);
for (int i = 1; i <= freeShapes.Length(); i++) {
    parseLabel(freeShapes.Value(i));
}
```

### 树结构构建

```cpp
struct STEPTreeNode {
    QString name;                    // 组件名称
    TopoDS_Shape shape;             // 几何形状
    TDF_Label stepLabel;            // STEP标签
    bool isVisible;                 // 可见性
    bool isAssembly;                // 是否装配体
    std::vector<std::shared_ptr<STEPTreeNode>> children;
};

void parseSTEPLabel(const TDF_Label& label, std::shared_ptr<STEPTreeNode> parent) {
    auto node = createNodeFromLabel(label);
    parent->children.push_back(node);
    
    if (shapeTool->IsAssembly(label)) {
        TDF_LabelSequence components;
        shapeTool->GetComponents(label, components);
        for (int i = 1; i <= components.Length(); i++) {
            parseSTEPLabel(components.Value(i), node);
        }
    }
}
```

### Qt模型集成

```cpp
class STEPModelTree : public QObject {
    Q_OBJECT
    
public:
    QStandardItemModel* getQtModel() const { return m_qtModel; }
    
private:
    void buildQtModelItem(std::shared_ptr<STEPTreeNode> node, QStandardItem* parentItem) {
        auto nameItem = new QStandardItem(node->name);
        nameItem->setCheckable(true);
        nameItem->setCheckState(node->isVisible ? Qt::Checked : Qt::Unchecked);
        
        auto typeItem = new QStandardItem(node->isAssembly ? "装配体" : "零件");
        parentItem->appendRow({nameItem, typeItem});
        
        for (auto& child : node->children) {
            buildQtModelItem(child, nameItem);
        }
    }
};
```

## 📋 功能特性

### 1. 层次结构显示

- **完整解析** - 支持多层嵌套装配体
- **类型识别** - 自动识别装配体和零件
- **名称提取** - 从STEP文件提取组件名称
- **标签显示** - 显示STEP内部标签信息

### 2. 可见性控制

- **单独控制** - 每个组件独立显示/隐藏
- **批量操作** - 全显示、全隐藏、仅显示选中
- **递归控制** - 装配体可见性影响子组件
- **实时更新** - 立即反映到3D视图

### 3. 交互功能

- **多选支持** - 支持Ctrl/Shift多选
- **右键菜单** - 丰富的上下文操作
- **拖拽排序** - 支持节点拖拽重排（可选）
- **搜索过滤** - 按名称快速查找

### 4. 显示选项

- **透明度控制** - 0-100%透明度调节
- **类型过滤** - 分别控制装配体/零件显示
- **颜色管理** - 支持STEP文件中的颜色信息
- **材质属性** - 显示材质和表面属性

## 🎯 使用场景

### 1. 复杂装配体分析

```cpp
// 场景：分析MPX3500机床的结构
modelTree->loadFromSTEPFile("MPX3500.step");

// 只显示主轴部分
auto spindleNodes = modelTree->findNodesByName("Spindle");
modelTree->setNodeVisibility(rootNode, false, true);  // 全隐藏
for (auto& node : spindleNodes) {
    modelTree->setNodeVisibility(node, true, true);   // 显示主轴
}
```

### 2. 零件检查

```cpp
// 场景：检查特定零件的安装位置
auto partNodes = modelTree->findNodesByName("HW14002270");
for (auto& node : partNodes) {
    // 高亮显示该零件
    highlightNode(node);
    // 显示零件路径
    QString path = modelTree->getNodePath(node);
    showPartInfo(path, node);
}
```

### 3. 装配体爆炸视图

```cpp
// 场景：创建装配体爆炸视图
void createExplodedView() {
    auto rootNode = modelTree->getRootNode();
    for (auto& child : rootNode->children) {
        if (child->isAssembly) {
            // 为每个子装配体设置偏移
            setAssemblyOffset(child, calculateOffset(child->level));
        }
    }
}
```

## 🔍 调试和诊断

### 模型统计信息

```cpp
auto stats = modelTree->getModelStats();
qDebug() << "模型统计:";
qDebug() << "  总节点数:" << stats.totalNodes;
qDebug() << "  装配体数:" << stats.assemblies;
qDebug() << "  零件数:" << stats.parts;
qDebug() << "  最大深度:" << stats.maxDepth;
qDebug() << "  可见节点:" << stats.visibleNodes;
```

### 性能监控

```cpp
// 加载性能监控
connect(modelTree, &STEPModelTree::loadProgress, [](int progress, const QString& msg) {
    qDebug() << QString("加载进度: %1% - %2").arg(progress).arg(msg);
});

// 内存使用监控
void monitorMemoryUsage() {
    auto shapes = modelTree->getVisibleShapes();
    qDebug() << "当前显示" << shapes.size() << "个形状";
    // 监控内存使用情况
}
```

## 🚀 扩展功能

### 1. 自定义属性

```cpp
// 扩展节点属性
struct ExtendedSTEPTreeNode : public STEPTreeNode {
    QColor customColor;
    double customTransparency;
    bool isHighlighted;
    QString materialName;
    QVariantMap userProperties;
};
```

### 2. 动画支持

```cpp
// 装配体动画
class AssemblyAnimator {
public:
    void animateAssembly(std::shared_ptr<STEPTreeNode> assembly);
    void createExplodeAnimation();
    void createRotationAnimation();
};
```

### 3. 导出功能

```cpp
// 导出选中组件
void exportSelectedComponents(const QString& filePath) {
    auto selectedNodes = getSelectedNodes();
    STEPCAFControl_Writer writer;
    for (auto& node : selectedNodes) {
        writer.WriteFile(node->shape, filePath);
    }
}
```

## 📝 配置选项

### CMakeLists.txt 配置

```cmake
# 添加STEP模型树支持
set(STEP_MODEL_TREE_SOURCES
    src/Data/STEPModelTree.h
    src/Data/STEPModelTree.cpp
    src/UI/ModelTreeDockWidget.h
    src/UI/ModelTreeDockWidget.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${STEP_MODEL_TREE_SOURCES})

# 确保OpenCASCADE XCAF支持
find_package(OpenCASCADE REQUIRED COMPONENTS TKXCAF TKXCAFSchema TKSTEP)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCASCADE_LIBRARIES})
```

### 编译选项

```cmake
# 启用STEP模型树功能
option(ENABLE_STEP_MODEL_TREE "Enable STEP model tree functionality" ON)

if(ENABLE_STEP_MODEL_TREE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_STEP_MODEL_TREE)
endif()
```

---

## 🎉 总结

STEP模型树功能为复杂CAD模型提供了专业级的管理界面，让用户能够：

- 🔍 **精确控制** - 对每个组件进行精确的显示控制
- 📊 **结构清晰** - 清楚地了解模型的层次结构  
- 🎛️ **操作便捷** - 提供直观的用户界面和丰富的操作
- 🚀 **性能优化** - 只渲染需要的组件，提升性能
- 🔧 **扩展性强** - 支持自定义功能和属性扩展

这个功能特别适合处理像MPX3500这样的复杂工业设备模型，让用户能够像使用专业CAD软件一样管理和查看模型结构。