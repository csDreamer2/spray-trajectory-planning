#include "STEPModelTree.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardItem>
#include <QDebug>

// OpenCASCADE includes
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>

// =============================================================================
// STEPModelTree Implementation
// =============================================================================

STEPModelTree::STEPModelTree(QObject* parent)
    : QObject(parent)
    , m_qtModel(new QStandardItemModel(this))
    , m_isLoading(false)
    , m_totalLabels(0)
    , m_processedLabels(0)
{
    // 设置Qt模型的列标题
    m_qtModel->setHorizontalHeaderLabels({
        tr("组件名称"), 
        tr("类型"), 
        tr("可见"), 
        tr("标签")
    });

    // 连接Qt模型的信号
    connect(m_qtModel, &QStandardItemModel::itemChanged,
            this, &STEPModelTree::onItemChanged);
}

STEPModelTree::~STEPModelTree() = default;

bool STEPModelTree::loadFromSTEPFile(const QString& filePath)
{
    if (m_isLoading) {
        qWarning() << "Already loading a STEP file";
        return false;
    }

    m_isLoading = true;
    emit loadProgress(0, tr("初始化STEP读取器..."));

    try {
        // 创建STEP读取器
        STEPCAFControl_Reader reader;
        
        // 创建XCAF文档
        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        app->NewDocument("MDTV-XCAF", m_stepDocument);
        
        // 设置读取器的文档
        reader.SetDocument(m_stepDocument);
        
        emit loadProgress(10, tr("读取STEP文件..."));
        
        // 读取STEP文件
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toLocal8Bit().constData());
        if (status != IFSelect_RetDone) {
            emit modelTreeLoaded(false, tr("无法读取STEP文件: %1").arg(filePath));
            m_isLoading = false;
            return false;
        }

        emit loadProgress(30, tr("传输数据到文档..."));
        
        // 传输数据到文档
        if (!reader.Transfer(m_stepDocument)) {
            emit modelTreeLoaded(false, tr("无法传输STEP数据到文档"));
            m_isLoading = false;
            return false;
        }

        emit loadProgress(50, tr("初始化XCAF工具..."));
        
        // 获取XCAF工具
        m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_stepDocument->Main());
        m_colorTool = XCAFDoc_DocumentTool::ColorTool(m_stepDocument->Main());
        m_layerTool = XCAFDoc_DocumentTool::LayerTool(m_stepDocument->Main());

        emit loadProgress(60, tr("解析模型结构..."));
        
        // 清除之前的数据
        m_rootNode.reset();
        m_labelToNode.clear();
        m_nameToNodes.clear();
        m_qtModel->clear();
        m_qtModel->setHorizontalHeaderLabels({
            tr("组件名称"), 
            tr("类型"), 
            tr("可见"), 
            tr("标签")
        });

        // 创建根节点
        m_rootNode = std::make_shared<STEPTreeNode>();
        m_rootNode->name = QFileInfo(filePath).baseName();
        m_rootNode->isAssembly = true;
        m_rootNode->level = 0;

        // 计算总标签数用于进度显示
        TDF_LabelSequence freeShapes;
        m_shapeTool->GetFreeShapes(freeShapes);
        m_totalLabels = freeShapes.Length();
        m_processedLabels = 0;

        emit loadProgress(70, tr("构建模型树..."));
        
        // 解析自由形状（顶级装配体）
        for (int i = 1; i <= freeShapes.Length(); i++) {
            TDF_Label label = freeShapes.Value(i);
            parseSTEPLabel(label, m_rootNode, 1);
        }

        emit loadProgress(90, tr("构建UI模型..."));
        
        // 构建Qt模型
        QStandardItem* rootItem = m_qtModel->invisibleRootItem();
        for (auto& child : m_rootNode->children) {
            buildQtModelItem(child, rootItem);
        }

        emit loadProgress(100, tr("加载完成"));
        emit modelTreeLoaded(true, tr("成功加载STEP文件，共 %1 个组件").arg(m_labelToNode.size()));
        
        m_isLoading = false;
        return true;

    } catch (const std::exception& e) {
        emit modelTreeLoaded(false, tr("加载STEP文件时发生异常: %1").arg(e.what()));
        m_isLoading = false;
        return false;
    }
}

void STEPModelTree::parseSTEPLabel(const TDF_Label& label, 
                                  std::shared_ptr<STEPTreeNode> parent, 
                                  int level)
{
    // 创建当前节点
    auto node = createNodeFromLabel(label, level);
    if (!node) {
        return;
    }

    // 设置父子关系
    node->parent = parent;
    parent->children.push_back(node);
    
    // 添加到映射表
    m_labelToNode[label] = node;
    m_nameToNodes[node->name].push_back(node);

    // 更新进度
    m_processedLabels++;
    if (m_totalLabels > 0) {
        int progress = 70 + (m_processedLabels * 20) / m_totalLabels;
        emit loadProgress(progress, tr("解析组件: %1").arg(node->name));
    }

    // 递归处理子标签
    if (m_shapeTool->IsAssembly(label)) {
        TDF_LabelSequence components;
        m_shapeTool->GetComponents(label, components);
        
        for (int i = 1; i <= components.Length(); i++) {
            TDF_Label componentLabel = components.Value(i);
            TDF_Label referredLabel;
            
            // 获取引用的标签
            if (m_shapeTool->GetReferredShape(componentLabel, referredLabel)) {
                parseSTEPLabel(referredLabel, node, level + 1);
            }
        }
    }
}

std::shared_ptr<STEPTreeNode> STEPModelTree::createNodeFromLabel(const TDF_Label& label, int level)
{
    auto node = std::make_shared<STEPTreeNode>();
    
    // 设置基本属性
    node->stepLabel = label;
    node->level = level;
    node->isVisible = true;
    
    // 获取名称
    node->name = getLabelName(label);
    if (node->name.isEmpty()) {
        // 如果没有名称，使用标签ID
        TCollection_AsciiString labelStr;
        TDF_Tool::Entry(label, labelStr);
        node->name = QString("Component_%1").arg(labelStr.ToCString());
    }

    // 检查是否为装配体
    node->isAssembly = isAssemblyLabel(label);
    
    // 获取形状
    TopoDS_Shape shape;
    if (m_shapeTool->GetShape(label, shape)) {
        node->shape = shape;
    }

    return node;
}

QString STEPModelTree::getLabelName(const TDF_Label& label) const
{
    Handle(TDataStd_Name) nameAttr;
    if (label.FindAttribute(TDataStd_Name::GetID(), nameAttr)) {
        TCollection_ExtendedString extName = nameAttr->Get();
        return QString::fromUtf16(reinterpret_cast<const ushort*>(extName.ToExtString()));
    }
    return QString();
}

bool STEPModelTree::isAssemblyLabel(const TDF_Label& label) const
{
    return m_shapeTool->IsAssembly(label);
}

void STEPModelTree::buildQtModelItem(std::shared_ptr<STEPTreeNode> node, QStandardItem* parentItem)
{
    // 创建主项（名称列）
    auto nameItem = new QStandardItem(node->name);
    nameItem->setCheckable(true);
    nameItem->setCheckState(node->isVisible ? Qt::Checked : Qt::Unchecked);
    nameItem->setData(QVariant::fromValue(node.get()), Qt::UserRole);

    // 创建类型列
    auto typeItem = new QStandardItem(node->isAssembly ? tr("装配体") : tr("零件"));
    
    // 创建可见性列
    auto visibilityItem = new QStandardItem(node->isVisible ? tr("显示") : tr("隐藏"));
    
    // 创建标签列
    TCollection_AsciiString labelStr;
    TDF_Tool::Entry(node->stepLabel, labelStr);
    auto labelItem = new QStandardItem(QString(labelStr.ToCString()));

    // 添加到父项
    parentItem->appendRow({nameItem, typeItem, visibilityItem, labelItem});

    // 递归处理子节点
    for (auto& child : node->children) {
        buildQtModelItem(child, nameItem);
    }
}

void STEPModelTree::setNodeVisibility(std::shared_ptr<STEPTreeNode> node, bool visible, bool recursive)
{
    if (!node) return;

    node->isVisible = visible;
    emit nodeVisibilityChanged(node, visible);

    if (recursive) {
        for (auto& child : node->children) {
            setNodeVisibility(child, visible, true);
        }
    }
}

std::vector<std::shared_ptr<STEPTreeNode>> STEPModelTree::findNodesByName(const QString& name) const
{
    auto it = m_nameToNodes.find(name);
    if (it != m_nameToNodes.end()) {
        return it->second;
    }
    return {};
}

std::vector<TopoDS_Shape> STEPModelTree::getVisibleShapes() const
{
    std::vector<TopoDS_Shape> shapes;
    if (m_rootNode) {
        collectVisibleShapes(m_rootNode, shapes);
    }
    return shapes;
}

void STEPModelTree::collectVisibleShapes(std::shared_ptr<STEPTreeNode> node, 
                                        std::vector<TopoDS_Shape>& shapes) const
{
    if (!node || !node->isVisible) {
        return;
    }

    // 如果节点有形状且不是装配体，添加到列表
    if (!node->shape.IsNull() && !node->isAssembly) {
        shapes.push_back(node->shape);
    }

    // 递归处理子节点
    for (auto& child : node->children) {
        collectVisibleShapes(child, shapes);
    }
}

QString STEPModelTree::getNodePath(std::shared_ptr<STEPTreeNode> node) const
{
    if (!node) return QString();

    QStringList path;
    auto current = node;
    
    while (current && current != m_rootNode) {
        path.prepend(current->name);
        current = current->parent.lock();
    }
    
    return path.join(" / ");
}

STEPModelTree::ModelStats STEPModelTree::getModelStats() const
{
    ModelStats stats = {};
    if (m_rootNode) {
        calculateStats(m_rootNode, stats);
    }
    return stats;
}

void STEPModelTree::calculateStats(std::shared_ptr<STEPTreeNode> node, ModelStats& stats) const
{
    if (!node) return;

    stats.totalNodes++;
    if (node->isVisible) stats.visibleNodes++;
    if (node->isAssembly) stats.assemblies++;
    else stats.parts++;
    
    if (node->level > stats.maxDepth) {
        stats.maxDepth = node->level;
    }

    for (auto& child : node->children) {
        calculateStats(child, stats);
    }
}

void STEPModelTree::onItemChanged(QStandardItem* item)
{
    if (!item || !item->isCheckable()) return;

    // 获取对应的节点
    auto nodePtr = item->data(Qt::UserRole).value<STEPTreeNode*>();
    if (!nodePtr) return;

    // 查找shared_ptr
    auto foundNode = findNodeByPointer(nodePtr);
    if (foundNode) {
        bool visible = (item->checkState() == Qt::Checked);
        setNodeVisibility(foundNode, visible, false);
    }
}

std::shared_ptr<STEPTreeNode> STEPModelTree::findNodeByPointer(STEPTreeNode* nodePtr) const
{
    if (!m_rootNode || !nodePtr) return nullptr;
    return findNodeInTreeByPointer(m_rootNode, nodePtr);
}

std::shared_ptr<STEPTreeNode> STEPModelTree::findNodeInTreeByPointer(
    std::shared_ptr<STEPTreeNode> current, STEPTreeNode* target) const
{
    if (!current || !target) return nullptr;
    
    if (current.get() == target) {
        return current;
    }
    
    for (auto& child : current->children) {
        auto found = findNodeInTreeByPointer(child, target);
        if (found) return found;
    }
    
    return nullptr;
}

// =============================================================================
// STEPModelTreeWidget Implementation
// =============================================================================

STEPModelTreeWidget::STEPModelTreeWidget(QWidget* parent)
    : QWidget(parent)
    , m_modelTree(new STEPModelTree(this))
    , m_treeView(new QTreeView(this))
{
    setupUI();
    setupContextMenu();

    // 连接信号
    connect(m_modelTree, &STEPModelTree::nodeVisibilityChanged,
            this, &STEPModelTreeWidget::onNodeVisibilityChanged);
    connect(m_modelTree, &STEPModelTree::modelTreeLoaded,
            this, &STEPModelTreeWidget::onModelTreeLoaded);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &STEPModelTreeWidget::onSelectionChanged);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &STEPModelTreeWidget::onContextMenuRequested);
}

STEPModelTreeWidget::~STEPModelTreeWidget() = default;

void STEPModelTreeWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // 配置树形视图
    m_treeView->setModel(m_modelTree->getQtModel());
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    m_layout->addWidget(m_treeView);
}

void STEPModelTreeWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_showAction = m_contextMenu->addAction(tr("显示"), [this]() {
        // TODO: 实现显示选中项
    });
    
    m_hideAction = m_contextMenu->addAction(tr("隐藏"), [this]() {
        // TODO: 实现隐藏选中项
    });
    
    m_showOnlyAction = m_contextMenu->addAction(tr("仅显示此项"), [this]() {
        // TODO: 实现仅显示选中项
    });
    
    m_contextMenu->addSeparator();
    
    m_expandAction = m_contextMenu->addAction(tr("展开所有"), [this]() {
        m_treeView->expandAll();
    });
    
    m_collapseAction = m_contextMenu->addAction(tr("折叠所有"), [this]() {
        m_treeView->collapseAll();
    });
    
    m_contextMenu->addSeparator();
    
    m_propertiesAction = m_contextMenu->addAction(tr("属性"), [this]() {
        // TODO: 显示属性对话框
    });
}

void STEPModelTreeWidget::loadSTEPFile(const QString& filePath)
{
    m_modelTree->loadFromSTEPFile(filePath);
}

void STEPModelTreeWidget::onSelectionChanged()
{
    std::vector<std::shared_ptr<STEPTreeNode>> selectedNodes;
    
    auto selection = m_treeView->selectionModel()->selectedIndexes();
    for (const auto& index : selection) {
        if (index.column() == 0) { // 只处理第一列
            auto item = m_modelTree->getQtModel()->itemFromIndex(index);
            auto node = getNodeFromItem(item);
            if (node) {
                selectedNodes.push_back(node);
            }
        }
    }
    
    emit selectionChanged(selectedNodes);
}

void STEPModelTreeWidget::onNodeVisibilityChanged(std::shared_ptr<STEPTreeNode> node, bool visible)
{
    Q_UNUSED(node)
    Q_UNUSED(visible)
    
    // 发送当前可见形状列表
    auto visibleShapes = m_modelTree->getVisibleShapes();
    emit visibilityChanged(visibleShapes);
}

void STEPModelTreeWidget::onModelTreeLoaded(bool success, const QString& message)
{
    if (success) {
        m_treeView->expandToDepth(1); // 展开第一层
        qDebug() << "STEP model tree loaded successfully:" << message;
    } else {
        QMessageBox::warning(this, tr("加载失败"), message);
    }
}

void STEPModelTreeWidget::onContextMenuRequested(const QPoint& pos)
{
    auto index = m_treeView->indexAt(pos);
    if (index.isValid()) {
        m_contextMenu->exec(m_treeView->mapToGlobal(pos));
    }
}

std::shared_ptr<STEPTreeNode> STEPModelTreeWidget::getNodeFromItem(QStandardItem* item) const
{
    if (!item) return nullptr;
    
    auto nodePtr = item->data(Qt::UserRole).value<STEPTreeNode*>();
    if (!nodePtr) return nullptr;
    
    // 从模型树中查找对应的shared_ptr
    // 注意：这里需要访问私有成员，所以需要友元声明或公共接口
    auto modelTree = m_modelTree;
    if (!modelTree) return nullptr;
    
    // 通过节点指针查找对应的shared_ptr
    auto rootNode = modelTree->getRootNode();
    if (!rootNode) return nullptr;
    
    return findNodeInTree(rootNode, nodePtr);
}

std::shared_ptr<STEPTreeNode> STEPModelTreeWidget::findNodeInTree(
    std::shared_ptr<STEPTreeNode> current, STEPTreeNode* target) const
{
    if (!current || !target) return nullptr;
    
    if (current.get() == target) {
        return current;
    }
    
    for (auto& child : current->children) {
        auto found = findNodeInTree(child, target);
        if (found) return found;
    }
    
    return nullptr;
}

#include "STEPModelTree.moc"
std::shared_ptr<STEPTreeNode> STEPModelTreeWidget::findNodeInTree(
    std::shared_ptr<STEPTreeNode> current, STEPTreeNode* target) const
{
    if (!current || !target) return nullptr;
    
    if (current.get() == target) {
        return current;
    }
    
    for (auto& child : current->children) {
        auto found = findNodeInTree(child, target);
        if (found) return found;
    }
    
    return nullptr;
}