#include "STEPModelTree.h"

#include <QStandardItem>
#include <QDebug>
#include <QFileInfo>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>

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

STEPModelTree::~STEPModelTree() 
{
    qDebug() << "STEPModelTree: 开始析构，清理OpenCASCADE资源...";
    
    try {
        // 清理映射表
        m_labelToNode.clear();
        m_nameToNodes.clear();
        
        // 清理节点树
        if (m_rootNode) {
            // 递归清理所有节点的OpenCASCADE引用
            clearNodeReferences(m_rootNode);
            m_rootNode.reset();
        }
        
        // 清理XCAF工具引用
        m_shapeTool.Nullify();
        m_colorTool.Nullify();
        m_layerTool.Nullify();
        
        // 清理STEP文档
        if (!m_stepDocument.IsNull()) {
            // 关闭文档
            Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
            if (!app.IsNull()) {
                app->Close(m_stepDocument);
            }
            m_stepDocument.Nullify();
        }
        
        qDebug() << "STEPModelTree: OpenCASCADE资源清理完成";
        
    } catch (const std::exception& e) {
        qWarning() << "STEPModelTree: 析构时异常:" << e.what();
    } catch (...) {
        qWarning() << "STEPModelTree: 析构时未知异常";
    }
    
    qDebug() << "STEPModelTree: 析构完成";
}

void STEPModelTree::clearNodeReferences(std::shared_ptr<STEPTreeNode> node)
{
    if (!node) return;
    
    try {
        // 清理OpenCASCADE引用
        if (!node->shape.IsNull()) {
            node->shape.Nullify();
        }
        
        // 递归清理子节点
        for (auto& child : node->children) {
            clearNodeReferences(child);
        }
        
        // 清理子节点列表
        node->children.clear();
        
    } catch (const std::exception& e) {
        qWarning() << "STEPModelTree: 清理节点引用时异常:" << e.what();
    } catch (...) {
        qWarning() << "STEPModelTree: 清理节点引用时未知异常";
    }
}

bool STEPModelTree::loadFromSTEPFile(const QString& filePath)
{
    if (m_isLoading) {
        qWarning() << "Already loading a STEP file";
        return false;
    }

    m_isLoading = true;
    emit loadProgress(0, tr("初始化STEP读取器..."));

    try {
        qDebug() << "STEPModelTree: Starting to load STEP file:" << filePath;
        
        // 检查文件是否存在
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            qCritical() << "STEPModelTree: File does not exist:" << filePath;
            emit modelTreeLoaded(false, tr("文件不存在: %1").arg(filePath));
            m_isLoading = false;
            return false;
        }
        
        qDebug() << "STEPModelTree: File size:" << fileInfo.size() << "bytes";
        
        // 创建STEP读取器
        qDebug() << "STEPModelTree: Creating STEP reader...";
        STEPCAFControl_Reader reader;
        
        // 创建XCAF文档
        qDebug() << "STEPModelTree: Creating XCAF document...";
        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        if (app.IsNull()) {
            qCritical() << "STEPModelTree: Failed to get XCAFApp_Application";
            emit modelTreeLoaded(false, tr("无法初始化XCAF应用程序"));
            m_isLoading = false;
            return false;
        }
        
        app->NewDocument("MDTV-XCAF", m_stepDocument);
        if (m_stepDocument.IsNull()) {
            qCritical() << "STEPModelTree: Failed to create XCAF document";
            emit modelTreeLoaded(false, tr("无法创建XCAF文档"));
            m_isLoading = false;
            return false;
        }
        
        emit loadProgress(10, tr("读取STEP文件..."));
        
        // 检查线程中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested before reading file";
            m_isLoading = false;
            return false;
        }
        
        // 读取STEP文件
        qDebug() << "STEPModelTree: Reading STEP file...";
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toLocal8Bit().constData());
        qDebug() << "STEPModelTree: Read status:" << (int)status;
        
        if (status != IFSelect_RetDone) {
            QString errorMsg;
            switch (status) {
                case IFSelect_RetError:
                    errorMsg = tr("读取文件时发生错误");
                    break;
                case IFSelect_RetFail:
                    errorMsg = tr("读取文件失败");
                    break;
                case IFSelect_RetVoid:
                    errorMsg = tr("文件为空或无效");
                    break;
                default:
                    errorMsg = tr("未知读取错误");
                    break;
            }
            qCritical() << "STEPModelTree: Failed to read STEP file:" << errorMsg;
            emit modelTreeLoaded(false, tr("无法读取STEP文件: %1 (%2)").arg(filePath).arg(errorMsg));
            m_isLoading = false;
            return false;
        }

        emit loadProgress(30, tr("传输数据到文档..."));
        
        // 检查线程中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested before transfer";
            m_isLoading = false;
            return false;
        }
        
        // 传输数据到文档 - 这是最容易阻塞的步骤
        qDebug() << "STEPModelTree: Transferring data to document...";
        
        // 使用QTimer延迟发送进度更新，避免信号阻塞
        QTimer::singleShot(0, this, [this]() {
            emit loadProgress(35, tr("正在传输STEP数据..."));
        });
        
        try {
            qDebug() << "STEPModelTree: 即将调用reader.Transfer()...";
            
            if (!reader.Transfer(m_stepDocument)) {
                qCritical() << "STEPModelTree: Failed to transfer STEP data to document";
                emit modelTreeLoaded(false, tr("无法传输STEP数据到文档"));
                m_isLoading = false;
                return false;
            }
            
            qDebug() << "STEPModelTree: reader.Transfer() 调用完成";
            
        } catch (const std::exception& e) {
            qWarning() << "STEPModelTree: Transfer异常:" << e.what();
            emit modelTreeLoaded(false, tr("数据传输异常: %1").arg(e.what()));
            m_isLoading = false;
            return false;
        } catch (...) {
            qWarning() << "STEPModelTree: Transfer未知异常";
            emit modelTreeLoaded(false, tr("数据传输发生未知异常"));
            m_isLoading = false;
            return false;
        }
        
        // 关键修复：多重线程状态刷新，确保Transfer完成后能继续
        qDebug() << "STEPModelTree: Transfer completed, applying thread state refresh...";
        
        // 第一次刷新
        QThread::msleep(10);
        QCoreApplication::processEvents();
        
        // 第二次刷新，确保信号队列被处理
        QThread::msleep(5);
        QCoreApplication::processEvents();
        
        // 使用定时器延迟发送后续信号，避免阻塞
        qDebug() << "STEPModelTree: Scheduling delayed progress update...";
        QTimer::singleShot(10, this, [this]() {
            qDebug() << "STEPModelTree: Delayed progress update - XCAF tools initialization";
            emit loadProgress(50, tr("初始化XCAF工具..."));
        });
        
        qDebug() << "STEPModelTree: Transfer completed, continuing to XCAF tools...";
        
        // 检查线程中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested before XCAF tools";
            m_isLoading = false;
            return false;
        }
        
        // 获取XCAF工具
        qDebug() << "STEPModelTree: Getting XCAF tools...";
        m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_stepDocument->Main());
        m_colorTool = XCAFDoc_DocumentTool::ColorTool(m_stepDocument->Main());
        m_layerTool = XCAFDoc_DocumentTool::LayerTool(m_stepDocument->Main());
        
        if (m_shapeTool.IsNull()) {
            qCritical() << "STEPModelTree: Failed to get ShapeTool";
            emit modelTreeLoaded(false, tr("无法获取XCAF形状工具"));
            m_isLoading = false;
            return false;
        }
        
        qDebug() << "STEPModelTree: XCAF tools initialized successfully";

        emit loadProgress(60, tr("解析模型结构..."));
        
        // 检查线程中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested before clearing data";
            m_isLoading = false;
            return false;
        }
        
        // 清除之前的数据
        qDebug() << "STEPModelTree: Clearing previous data...";
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
        qDebug() << "STEPModelTree: Creating root node...";
        m_rootNode = std::make_shared<STEPTreeNode>();
        m_rootNode->name = QFileInfo(filePath).baseName();
        m_rootNode->isAssembly = true;
        m_rootNode->level = 0;

        // 计算总标签数用于进度显示
        qDebug() << "STEPModelTree: Getting free shapes...";
        TDF_LabelSequence freeShapes;
        m_shapeTool->GetFreeShapes(freeShapes);
        m_totalLabels = freeShapes.Length();
        m_processedLabels = 0;
        
        qDebug() << "STEPModelTree: Found" << m_totalLabels << "free shapes";

        emit loadProgress(70, tr("构建模型树..."));
        
        // 添加递归深度限制，防止栈溢出
        const int MAX_RECURSION_DEPTH = 100;
        
        // 解析自由形状（顶级装配体）
        qDebug() << "STEPModelTree: Starting to parse free shapes...";
        for (int i = 1; i <= freeShapes.Length(); i++) {
            // 检查线程中断
            if (QThread::currentThread()->isInterruptionRequested()) {
                qDebug() << "STEPModelTree: Thread interruption requested during free shapes parsing";
                m_isLoading = false;
                return false;
            }
            
            qDebug() << "STEPModelTree: Parsing free shape" << i << "of" << freeShapes.Length();
            
            try {
                TDF_Label label = freeShapes.Value(i);
                
                // 添加try-catch保护每个标签的解析
                parseSTEPLabel(label, m_rootNode, 1, MAX_RECURSION_DEPTH);
                
                qDebug() << "STEPModelTree: Successfully parsed free shape" << i;
                
            } catch (const std::exception& e) {
                qWarning() << "STEPModelTree: Exception parsing free shape" << i << ":" << e.what();
                // 继续处理其他形状
            } catch (...) {
                qWarning() << "STEPModelTree: Unknown exception parsing free shape" << i;
                // 继续处理其他形状
            }
        }
        
        qDebug() << "STEPModelTree: Finished parsing all free shapes";

        // 最后检查中断
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested before UI model building";
            m_isLoading = false;
            return false;
        }

        emit loadProgress(90, tr("构建UI模型..."));
        
        // 构建Qt模型 - 添加异常保护
        try {
            qDebug() << "STEPModelTree: Building Qt model with" << m_rootNode->children.size() << "root children";
            
            QStandardItem* rootItem = m_qtModel->invisibleRootItem();
            for (auto& child : m_rootNode->children) {
                try {
                    buildQtModelItem(child, rootItem);
                } catch (const std::exception& e) {
                    qWarning() << "STEPModelTree: Exception building Qt model item:" << e.what();
                    // 继续处理其他子项
                } catch (...) {
                    qWarning() << "STEPModelTree: Unknown exception building Qt model item";
                    // 继续处理其他子项
                }
            }
            
            qDebug() << "STEPModelTree: Qt model built successfully";
            
        } catch (const std::exception& e) {
            qWarning() << "STEPModelTree: Exception building Qt model:" << e.what();
            emit modelTreeLoaded(false, tr("构建UI模型时发生异常: %1").arg(e.what()));
            m_isLoading = false;
            return false;
        } catch (...) {
            qWarning() << "STEPModelTree: Unknown exception building Qt model";
            emit modelTreeLoaded(false, tr("构建UI模型时发生未知异常"));
            m_isLoading = false;
            return false;
        }

        // 强制刷新线程状态，防止信号发送阻塞
        QThread::msleep(5);
        QCoreApplication::processEvents();

        emit loadProgress(100, tr("加载完成"));
        
        qDebug() << "STEPModelTree: Sending completion signal...";
        emit modelTreeLoaded(true, tr("成功加载STEP文件，共 %1 个组件").arg(m_labelToNode.size()));
        
        qDebug() << "STEPModelTree: Load completed successfully";
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
                                  int level,
                                  int maxDepth)
{
    try {
        // 检查递归深度限制，防止栈溢出
        if (level > maxDepth) {
            qWarning() << "STEPModelTree: Maximum recursion depth reached at level" << level << ", skipping deeper levels";
            return;
        }
        
        // 检查线程中断请求
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "STEPModelTree: Thread interruption requested, stopping parseSTEPLabel at level" << level;
            return;
        }

        // 每处理一定数量的节点就让出CPU时间
        static int processedCount = 0;
        processedCount++;
        if (processedCount % 50 == 0) {
            QThread::msleep(1); // 让出1ms CPU时间
        }

        qDebug() << "STEPModelTree: Parsing label at level" << level << "(depth limit:" << maxDepth << ")";

        // 创建当前节点
        auto node = createNodeFromLabel(label, level);
        if (!node) {
            qWarning() << "STEPModelTree: Failed to create node from label at level" << level;
            return;
        }

        qDebug() << "STEPModelTree: Created node:" << node->name << "at level" << level;

        // 设置父子关系
        node->parent = parent;
        parent->children.push_back(node);
        
        // 添加到映射表
        TCollection_AsciiString labelStr;
        TDF_Tool::Entry(label, labelStr);
        std::string labelKey = labelStr.ToCString();
        m_labelToNode[labelKey] = node;
        m_nameToNodes[node->name].push_back(node);

        // 更新进度
        m_processedLabels++;
        if (m_totalLabels > 0) {
            int progress = 70 + (m_processedLabels * 20) / m_totalLabels;
            emit loadProgress(progress, tr("解析组件: %1 (层级: %2)").arg(node->name).arg(level));
        }

        // 递归处理子标签
        if (m_shapeTool && m_shapeTool->IsAssembly(label)) {
            qDebug() << "STEPModelTree: Node is assembly, getting components...";
            
            TDF_LabelSequence components;
            m_shapeTool->GetComponents(label, components);
            
            qDebug() << "STEPModelTree: Found" << components.Length() << "components in assembly";
            
            for (int i = 1; i <= components.Length(); i++) {
                // 每处理几个组件检查一次中断
                if (i % 5 == 0 && QThread::currentThread()->isInterruptionRequested()) {
                    qDebug() << "STEPModelTree: Thread interruption requested during component processing at level" << level;
                    return;
                }
                
                try {
                    TDF_Label componentLabel = components.Value(i);
                    TDF_Label referredLabel;
                    
                    // 获取引用的标签
                    if (m_shapeTool->GetReferredShape(componentLabel, referredLabel)) {
                        qDebug() << "STEPModelTree: Processing component" << i << "at level" << (level + 1);
                        parseSTEPLabel(referredLabel, node, level + 1, maxDepth);
                    }
                } catch (const std::exception& e) {
                    qWarning() << "STEPModelTree: Exception processing component" << i << "at level" << level << ":" << e.what();
                    // 继续处理其他组件
                } catch (...) {
                    qWarning() << "STEPModelTree: Unknown exception processing component" << i << "at level" << level;
                    // 继续处理其他组件
                }
            }
        }
        
        qDebug() << "STEPModelTree: Finished processing node at level" << level;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTree: Exception in parseSTEPLabel at level" << level << ":" << e.what();
        // 不重新抛出异常，继续处理其他节点
    } catch (...) {
        qCritical() << "STEPModelTree: Unknown exception in parseSTEPLabel at level" << level;
        // 不重新抛出异常，继续处理其他节点
    }
}

std::shared_ptr<STEPTreeNode> STEPModelTree::createNodeFromLabel(const TDF_Label& label, int level)
{
    try {
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
        if (m_shapeTool) {
            node->isAssembly = isAssemblyLabel(label);
        } else {
            qWarning() << "STEPModelTree: m_shapeTool is null in createNodeFromLabel";
            node->isAssembly = false;
        }
        
        // 获取形状
        TopoDS_Shape shape;
        if (m_shapeTool && m_shapeTool->GetShape(label, shape)) {
            node->shape = shape;
        }

        return node;
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTree: Exception in createNodeFromLabel:" << e.what();
        return nullptr;
    } catch (...) {
        qCritical() << "STEPModelTree: Unknown exception in createNodeFromLabel";
        return nullptr;
    }
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
    try {
        if (!node || !parentItem) {
            qWarning() << "STEPModelTree: Invalid node or parent item in buildQtModelItem";
            return;
        }
        
        // 创建主项（名称列）
        auto nameItem = new QStandardItem(node->name);
        nameItem->setCheckable(true);
        nameItem->setCheckState(node->isVisible ? Qt::Checked : Qt::Unchecked);
        nameItem->setData(QVariant::fromValue(node.get()), Qt::UserRole);

        // 创建类型列
        auto typeItem = new QStandardItem(node->isAssembly ? tr("装配体") : tr("零件"));
        
        // 创建可见性列
        auto visibilityItem = new QStandardItem(node->isVisible ? tr("显示") : tr("隐藏"));
        
        // 创建标签列 - 添加保护
        QString labelText;
        try {
            if (!node->stepLabel.IsNull()) {
                TCollection_AsciiString labelStr;
                TDF_Tool::Entry(node->stepLabel, labelStr);
                labelText = QString(labelStr.ToCString());
            } else {
                labelText = QString("Level_%1").arg(node->level);
            }
        } catch (...) {
            labelText = QString("Level_%1").arg(node->level);
        }
        auto labelItem = new QStandardItem(labelText);

        // 添加到父项
        parentItem->appendRow({nameItem, typeItem, visibilityItem, labelItem});

        // 递归处理子节点 - 添加保护
        for (auto& child : node->children) {
            try {
                buildQtModelItem(child, nameItem);
            } catch (const std::exception& e) {
                qWarning() << "STEPModelTree: Exception in recursive buildQtModelItem:" << e.what();
                // 继续处理其他子节点
            } catch (...) {
                qWarning() << "STEPModelTree: Unknown exception in recursive buildQtModelItem";
                // 继续处理其他子节点
            }
        }
        
    } catch (const std::exception& e) {
        qWarning() << "STEPModelTree: Exception in buildQtModelItem:" << e.what();
    } catch (...) {
        qWarning() << "STEPModelTree: Unknown exception in buildQtModelItem";
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