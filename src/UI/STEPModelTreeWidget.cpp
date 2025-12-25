#include "STEPModelTreeWidget.h"
#include "../Data/STEPModelTreeWorker.h"

#include <QApplication>
#include <QHeaderView>
#include <QStandardItem>
#include <QMessageBox>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

STEPModelTreeWidget::STEPModelTreeWidget(QWidget* parent)
    : QWidget(parent)
    , m_modelTree(new STEPModelTree(this))
    , m_treeView(new QTreeView(this))
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_isLoading(false)
    , m_loadedRootNode(nullptr)  // 初始化根节点为空
{
    setupUI();
    setupContextMenu();
    setupWorker();

    // 连接信号
    connect(m_modelTree, &STEPModelTree::nodeVisibilityChanged,
            this, &STEPModelTreeWidget::onNodeVisibilityChanged);
    connect(m_modelTree, &STEPModelTree::modelTreeLoaded,
            this, &STEPModelTreeWidget::onModelTreeLoaded);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &STEPModelTreeWidget::onSelectionChanged);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &STEPModelTreeWidget::onContextMenuRequested);
    
    // 连接Qt模型的itemChanged信号，监听复选框变化
    connect(m_modelTree->getQtModel(), &QStandardItemModel::itemChanged,
            this, &STEPModelTreeWidget::onItemChanged);
}

STEPModelTreeWidget::~STEPModelTreeWidget() 
{
    qDebug() << "STEPModelTreeWidget: 开始析构...";
    
    // 确保线程被正确清理
    if (m_workerThread) {
        qDebug() << "STEPModelTreeWidget: 清理工作线程...";
        
        // 如果正在加载，先取消
        if (m_isLoading) {
            qDebug() << "STEPModelTreeWidget: 取消正在进行的加载...";
            m_isLoading = false;
        }
        
        // 请求线程中断
        m_workerThread->requestInterruption();
        
        // 退出线程事件循环
        m_workerThread->quit();
        
        // 等待线程结束（最多3秒）
        if (!m_workerThread->wait(3000)) {
            qWarning() << "STEPModelTreeWidget: 工作线程未能在3秒内结束，强制终止...";
            m_workerThread->terminate();
            
            // 再等待1秒让terminate生效
            if (!m_workerThread->wait(1000)) {
                qCritical() << "STEPModelTreeWidget: 工作线程强制终止失败！";
            }
        }
        
        qDebug() << "STEPModelTreeWidget: 工作线程已清理";
        
        // 注意：不要手动删除m_worker，它会通过finished信号自动删除
        // 也不要手动删除m_workerThread，它是this的子对象会自动删除
    }
    
    // 清理模型数据
    if (m_modelTree) {
        qDebug() << "STEPModelTreeWidget: 清理模型树数据...";
        // STEPModelTree会在其析构函数中清理OpenCASCADE对象
    }
    
    qDebug() << "STEPModelTreeWidget: 析构完成";
}

void STEPModelTreeWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // 添加状态标签
    m_statusLabel = new QLabel("准备加载STEP模型...", this);
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; padding: 4px; }");
    m_layout->addWidget(m_statusLabel);

    // 添加进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(true);
    m_layout->addWidget(m_progressBar);

    // 配置树形视图
    m_treeView->setModel(m_modelTree->getQtModel());
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->header()->setStretchLastSection(false);
    
    // 设置列宽：组件名称列更宽，其他列固定宽度
    m_treeView->header()->setSectionResizeMode(0, QHeaderView::Interactive); // 组件名称列可调整
    m_treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // 类型列自适应
    m_treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // 可见列自适应
    m_treeView->header()->setSectionResizeMode(3, QHeaderView::Stretch); // 标签列拉伸填充
    
    // 设置初始列宽
    m_treeView->setColumnWidth(0, 250); // 组件名称列宽度为250像素
    m_treeView->setColumnWidth(1, 80);  // 类型列宽度为80像素
    m_treeView->setColumnWidth(2, 60);  // 可见列宽度为60像素
    
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
    if (m_isLoading) {
        qWarning() << "STEPModelTreeWidget: Already loading a file";
        return;
    }

    qDebug() << "=== MAIN THREAD: Starting async STEP model tree loading ===" << filePath;
    
    m_isLoading = true;
    showLoadingUI(true);
    
    // 清除当前模型
    m_modelTree->getQtModel()->clear();
    m_modelTree->getQtModel()->setHorizontalHeaderLabels({
        tr("组件名称"), 
        tr("类型"), 
        tr("可见"), 
        tr("标签")
    });
    
    // 发送加载请求到工作线程
    QMetaObject::invokeMethod(m_worker, "loadSTEPFile", Qt::QueuedConnection,
                              Q_ARG(QString, filePath));
}

void STEPModelTreeWidget::cancelLoading()
{
    if (m_isLoading) {
        qDebug() << "Cancelling STEP model tree loading...";
        m_isLoading = false;
        showLoadingUI(false);
    }
    
    if (m_workerThread && m_workerThread->isRunning()) {
        qDebug() << "Stopping worker thread...";
        
        // 请求线程中断
        m_workerThread->requestInterruption();
        
        // 退出线程事件循环
        m_workerThread->quit();
        
        // 等待线程结束（最多3秒）
        if (!m_workerThread->wait(3000)) {
            qWarning() << "Worker thread did not finish in time, terminating...";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }
        
        qDebug() << "Worker thread stopped successfully";
    }
}

void STEPModelTreeWidget::setupWorker()
{
    // 创建工作线程
    m_workerThread = new QThread(this);
    m_worker = new STEPModelTreeWorker();
    
    // 移动Worker到线程
    m_worker->moveToThread(m_workerThread);
    
    // 连接信号槽
    connect(m_worker, &STEPModelTreeWorker::progressUpdate,
            this, &STEPModelTreeWidget::onWorkerProgressUpdate, Qt::QueuedConnection);
    connect(m_worker, &STEPModelTreeWorker::modelTreeLoaded,
            this, &STEPModelTreeWidget::onWorkerModelTreeLoaded, Qt::QueuedConnection);
    connect(m_worker, &STEPModelTreeWorker::loadFailed,
            this, &STEPModelTreeWidget::onWorkerLoadFailed, Qt::QueuedConnection);
    
    // 连接线程清理信号
    connect(m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    
    // 启动线程
    m_workerThread->start(QThread::HighPriority);
    
    qDebug() << "STEPModelTreeWidget: Worker thread started";
}

void STEPModelTreeWidget::showLoadingUI(bool show)
{
    m_progressBar->setVisible(show);
    if (show) {
        m_statusLabel->setText("正在加载STEP模型...");
        m_progressBar->setValue(0);
        m_treeView->setEnabled(false);
    } else {
        m_progressBar->setValue(100);
        m_treeView->setEnabled(true);
    }
}

void STEPModelTreeWidget::onWorkerProgressUpdate(int progress, const QString& message)
{
    m_progressBar->setValue(progress);
    m_statusLabel->setText(message);
    QApplication::processEvents(); // 确保UI更新
}

void STEPModelTreeWidget::onWorkerModelTreeLoaded(bool success, const QString& message, std::shared_ptr<STEPTreeNode> rootNode)
{
    qDebug() << "STEPModelTreeWidget: 接收到工作线程完成信号，success=" << success;
    
    // 使用QTimer延迟处理，避免直接在信号槽中处理复杂操作
    QTimer::singleShot(0, this, [this, success, message, rootNode]() {
        try {
            qDebug() << "STEPModelTreeWidget: 开始延迟处理完成信号...";
            
            m_isLoading = false;
            showLoadingUI(false);
            
            if (success && rootNode) {
                qDebug() << "STEPModelTreeWidget: 处理成功结果...";
                m_statusLabel->setText("STEP模型树加载成功");
                
                // 分步骤更新，每步之间给UI时间响应
                QTimer::singleShot(10, this, [this, rootNode]() {
                    try {
                        qDebug() << "STEPModelTreeWidget: 开始更新模型数据...";
                        updateModelFromWorkerResult(rootNode);
                        
                        // 再次延迟UI更新
                        QTimer::singleShot(100, this, [this]() {
                            try {
                                qDebug() << "STEPModelTreeWidget: 开始UI更新...";
                                // 暂时禁用自动展开，避免崩溃
                                // QCoreApplication::processEvents();
                                // m_treeView->expandToDepth(1);
                                // QCoreApplication::processEvents();
                                qDebug() << "STEPModelTreeWidget: UI更新完成（跳过自动展开）";
                            } catch (const std::exception& e) {
                                qCritical() << "STEPModelTreeWidget: UI更新异常:" << e.what();
                            } catch (...) {
                                qCritical() << "STEPModelTreeWidget: UI更新未知异常";
                            }
                        });
                        
                    } catch (const std::exception& e) {
                        qCritical() << "STEPModelTreeWidget: 模型更新异常:" << e.what();
                        m_statusLabel->setText("模型更新失败");
                        emit loadCompleted(false, QString("模型更新异常: %1").arg(e.what()));
                        return;
                    } catch (...) {
                        qCritical() << "STEPModelTreeWidget: 模型更新未知异常";
                        m_statusLabel->setText("模型更新失败");
                        emit loadCompleted(false, "模型更新发生未知异常");
                        return;
                    }
                });
                
                qDebug() << "=== MAIN THREAD: STEP model tree loading completed successfully ===";
                
                // 延迟发送完成信号，确保所有UI更新完成
                QTimer::singleShot(200, this, [this, message]() {
                    qDebug() << "STEPModelTreeWidget: 发送加载完成信号";
                    emit loadCompleted(true, message);
                });
                
            } else {
                qDebug() << "STEPModelTreeWidget: 处理失败结果...";
                m_statusLabel->setText("STEP模型树加载失败");
                
                // 延迟显示错误消息，避免在信号处理中直接显示对话框
                QTimer::singleShot(10, this, [this, message]() {
                    QMessageBox::warning(this, tr("加载失败"), message);
                    emit loadCompleted(false, message);
                });
            }
            
        } catch (const std::exception& e) {
            qCritical() << "STEPModelTreeWidget: 完成信号处理异常:" << e.what();
            m_isLoading = false;
            showLoadingUI(false);
            m_statusLabel->setText("处理完成信号时发生异常");
            
            QTimer::singleShot(10, this, [this, e]() {
                emit loadCompleted(false, QString("信号处理异常: %1").arg(e.what()));
            });
            
        } catch (...) {
            qCritical() << "STEPModelTreeWidget: 完成信号处理未知异常";
            m_isLoading = false;
            showLoadingUI(false);
            m_statusLabel->setText("处理完成信号时发生未知异常");
            
            QTimer::singleShot(10, this, [this]() {
                emit loadCompleted(false, "信号处理发生未知异常");
            });
        }
    });
}

void STEPModelTreeWidget::onWorkerLoadFailed(const QString& error)
{
    m_isLoading = false;
    showLoadingUI(false);
    m_statusLabel->setText("加载失败");
    
    QMessageBox::critical(this, tr("加载错误"), error);
    emit loadCompleted(false, error);
}

void STEPModelTreeWidget::updateModelFromWorkerResult(std::shared_ptr<STEPTreeNode> rootNode)
{
    qDebug() << "STEPModelTreeWidget: 开始更新模型数据...";
    
    try {
        // 重要：先更新STEPModelTree的根节点
        if (m_modelTree) {
            qDebug() << "STEPModelTreeWidget: 设置模型树根节点";
            // 直接访问m_modelTree的私有成员需要添加友元或公共方法
            // 这里我们通过重新加载来设置根节点
            // 但由于是异步加载，我们需要另一种方式
        }
        
        // 清除当前模型
        m_modelTree->getQtModel()->clear();
        m_modelTree->getQtModel()->setHorizontalHeaderLabels({
            tr("组件名称"), 
            tr("类型"), 
            tr("可见"), 
            tr("标签")
        });
        
        // 递归构建Qt模型项
        if (rootNode) {
            qDebug() << "STEPModelTreeWidget: 根节点有" << rootNode->children.size() << "个子节点";
            
            QStandardItem* rootItem = m_modelTree->getQtModel()->invisibleRootItem();
            
            // 分批处理子节点，避免一次性处理太多导致崩溃
            int processedCount = 0;
            for (auto& child : rootNode->children) {
                try {
                    buildQtModelItemFromNode(child, rootItem);
                    processedCount++;
                    
                    // 每处理10个节点就让UI响应一下
                    if (processedCount % 10 == 0) {
                        QCoreApplication::processEvents();
                        QThread::msleep(1);
                    }
                    
                } catch (const std::exception& e) {
                    qWarning() << "STEPModelTreeWidget: 构建子节点异常:" << e.what();
                    // 继续处理其他子节点
                } catch (...) {
                    qWarning() << "STEPModelTreeWidget: 构建子节点未知异常";
                    // 继续处理其他子节点
                }
            }
            
            qDebug() << "STEPModelTreeWidget: 成功处理了" << processedCount << "个子节点";
            
            // 重要：保存根节点引用供后续使用
            m_loadedRootNode = rootNode;
            qDebug() << "STEPModelTreeWidget: 已保存根节点引用";
        }
        
        qDebug() << "STEPModelTreeWidget: 模型数据更新完成";
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: updateModelFromWorkerResult异常:" << e.what();
        throw; // 重新抛出让上层处理
    } catch (...) {
        qCritical() << "STEPModelTreeWidget: updateModelFromWorkerResult未知异常";
        throw; // 重新抛出让上层处理
    }
}

void STEPModelTreeWidget::buildQtModelItemFromNode(std::shared_ptr<STEPTreeNode> node, QStandardItem* parentItem)
{
    if (!node) {
        qWarning() << "STEPModelTreeWidget: buildQtModelItemFromNode收到空节点";
        return;
    }
    
    if (!parentItem) {
        qWarning() << "STEPModelTreeWidget: buildQtModelItemFromNode收到空父项";
        return;
    }
    
    try {
        // 创建主项（名称列）- 添加保护
        QString nodeName = node->name.isEmpty() ? "未命名组件" : node->name;
        auto nameItem = new QStandardItem(nodeName);
        nameItem->setCheckable(true);
        nameItem->setCheckState(node->isVisible ? Qt::Checked : Qt::Unchecked);
        
        // 安全地存储节点指针 - 使用quintptr转换
        try {
            quintptr nodePtr = reinterpret_cast<quintptr>(node.get());
            nameItem->setData(QVariant::fromValue(nodePtr), Qt::UserRole);
            qDebug() << "STEPModelTreeWidget: 存储节点指针:" << nodeName << "ptr=" << nodePtr;
        } catch (...) {
            qWarning() << "STEPModelTreeWidget: 无法设置节点数据，跳过";
        }

        // 创建类型列
        auto typeItem = new QStandardItem(node->isAssembly ? tr("装配体") : tr("零件"));
        
        // 创建可见性列
        auto visibilityItem = new QStandardItem(node->isVisible ? tr("显示") : tr("隐藏"));
        
        // 创建标签列
        auto labelItem = new QStandardItem(QString("Level_%1").arg(node->level));

        // 添加到父项 - 添加保护
        try {
            parentItem->appendRow({nameItem, typeItem, visibilityItem, labelItem});
        } catch (const std::exception& e) {
            qWarning() << "STEPModelTreeWidget: 添加行到模型异常:" << e.what();
            // 清理已创建的项
            delete nameItem;
            delete typeItem;
            delete visibilityItem;
            delete labelItem;
            return;
        } catch (...) {
            qWarning() << "STEPModelTreeWidget: 添加行到模型未知异常";
            // 清理已创建的项
            delete nameItem;
            delete typeItem;
            delete visibilityItem;
            delete labelItem;
            return;
        }

        // 递归处理子节点 - 限制递归深度防止栈溢出
        if (node->level < 50) { // 限制最大深度为50
            for (auto& child : node->children) {
                try {
                    buildQtModelItemFromNode(child, nameItem);
                } catch (const std::exception& e) {
                    qWarning() << "STEPModelTreeWidget: 递归构建子节点异常:" << e.what();
                    // 继续处理其他子节点
                } catch (...) {
                    qWarning() << "STEPModelTreeWidget: 递归构建子节点未知异常";
                    // 继续处理其他子节点
                }
            }
        } else {
            qWarning() << "STEPModelTreeWidget: 达到最大递归深度，跳过子节点";
        }
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: buildQtModelItemFromNode异常:" << e.what();
    } catch (...) {
        qCritical() << "STEPModelTreeWidget: buildQtModelItemFromNode未知异常";
    }
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
    if (!item) {
        qWarning() << "STEPModelTreeWidget: getNodeFromItem收到空item";
        return nullptr;
    }
    
    // 从UserRole获取节点指针
    QVariant data = item->data(Qt::UserRole);
    if (!data.isValid()) {
        qWarning() << "STEPModelTreeWidget: Item没有存储节点数据:" << item->text();
        return nullptr;
    }
    
    // 转换quintptr回指针
    quintptr nodePtr = data.value<quintptr>();
    if (nodePtr == 0) {
        qWarning() << "STEPModelTreeWidget: 节点指针为空:" << item->text();
        return nullptr;
    }
    
    STEPTreeNode* rawPtr = reinterpret_cast<STEPTreeNode*>(nodePtr);
    
    // 从加载的根节点中查找对应的shared_ptr
    if (!m_loadedRootNode) {
        qWarning() << "STEPModelTreeWidget: 加载的根节点为空";
        return nullptr;
    }
    
    auto sharedNode = findNodeInTree(m_loadedRootNode, rawPtr);
    if (!sharedNode) {
        qWarning() << "STEPModelTreeWidget: 无法在树中找到节点:" << item->text();
    }
    
    return sharedNode;
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

void STEPModelTreeWidget::onItemChanged(QStandardItem* item)
{
    if (!item || !item->isCheckable()) return;
    
    // 获取对应的节点
    auto node = getNodeFromItem(item);
    if (!node) {
        qWarning() << "STEPModelTreeWidget: 无法从Item获取节点";
        return;
    }
    
    // 获取新的可见状态
    bool visible = (item->checkState() == Qt::Checked);
    
    qDebug() << "STEPModelTreeWidget: 节点可见性变化:" << node->name << "可见:" << visible;
    
    // 更新节点的可见状态
    node->isVisible = visible;
    
    // 如果节点有Actor，更新其可见性
    if (node->actor) {
        node->actor->SetVisibility(visible ? 1 : 0);
        qDebug() << "STEPModelTreeWidget: 已更新Actor可见性";
    }
    
    // 递归更新子节点
    setChildrenVisibility(item, visible);
    
    // 延迟发送信号，避免频繁触发渲染
    // 使用QTimer合并多个快速变化
    static QTimer* renderTimer = nullptr;
    if (!renderTimer) {
        renderTimer = new QTimer(this);
        renderTimer->setSingleShot(true);
        renderTimer->setInterval(100);  // 100ms延迟
        connect(renderTimer, &QTimer::timeout, this, [this, node, visible]() {
            emit nodeVisibilityToggled(node, visible);
        });
    }
    
    // 重启定时器（如果已经在运行，会重新计时）
    renderTimer->start();
}

void STEPModelTreeWidget::setChildrenVisibility(QStandardItem* item, bool visible)
{
    if (!item) return;
    
    // 递归处理所有子项
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem* child = item->child(i, 0);  // 第一列是名称列
        if (child && child->isCheckable()) {
            // 暂时断开信号，避免递归触发
            disconnect(m_modelTree->getQtModel(), &QStandardItemModel::itemChanged,
                       this, &STEPModelTreeWidget::onItemChanged);
            
            // 设置子项的勾选状态
            child->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
            
            // 重新连接信号
            connect(m_modelTree->getQtModel(), &QStandardItemModel::itemChanged,
                    this, &STEPModelTreeWidget::onItemChanged);
            
            // 更新对应节点的Actor
            auto childNode = getNodeFromItem(child);
            if (childNode) {
                childNode->isVisible = visible;
                if (childNode->actor) {
                    childNode->actor->SetVisibility(visible ? 1 : 0);
                }
            }
            
            // 递归处理子节点的子节点
            setChildrenVisibility(child, visible);
        }
    }
}