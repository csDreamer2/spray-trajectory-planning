#include "ModelTreeDockWidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

ModelTreeDockWidget::ModelTreeDockWidget(QWidget* parent)
    : QDockWidget(tr("模型树"), parent)
    , m_modelTreeWidget(nullptr)
    , m_isLoading(false)
    , m_totalNodes(0)
    , m_visibleNodes(0)
    , m_selectedNodes(0)
{
    setupUI();

    // 连接信号
    connect(m_modelTreeWidget, &STEPModelTreeWidget::visibilityChanged,
            this, &ModelTreeDockWidget::onVisibilityChanged);
    connect(m_modelTreeWidget, &STEPModelTreeWidget::selectionChanged,
            this, &ModelTreeDockWidget::onSelectionChanged);
    connect(m_modelTreeWidget->getModelTree(), &STEPModelTree::modelTreeLoaded,
            this, &ModelTreeDockWidget::onModelTreeLoaded);
    connect(m_modelTreeWidget->getModelTree(), &STEPModelTree::loadProgress,
            this, &ModelTreeDockWidget::onLoadProgress);
}

ModelTreeDockWidget::~ModelTreeDockWidget() = default;

void ModelTreeDockWidget::setupUI()
{
    // 创建主窗口部件
    m_mainWidget = new QWidget(this);
    m_mainLayout = new QVBoxLayout(m_mainWidget);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);

    // 创建分割器
    m_splitter = new QSplitter(Qt::Vertical, this);

    // 创建模型树控件
    m_modelTreeWidget = new STEPModelTreeWidget(this);
    m_splitter->addWidget(m_modelTreeWidget);

    // 设置控制面板
    setupControlPanel();
    setupDisplayOptions();

    // 创建状态信息组
    m_statusGroup = new QGroupBox(tr("状态信息"), this);
    m_statusLayout = new QVBoxLayout(m_statusGroup);

    m_totalNodesLabel = new QLabel(tr("总节点: 0"), this);
    m_visibleNodesLabel = new QLabel(tr("可见节点: 0"), this);
    m_selectedNodesLabel = new QLabel(tr("选中节点: 0"), this);
    
    m_loadProgressBar = new QProgressBar(this);
    m_loadProgressBar->setVisible(false);
    m_loadStatusLabel = new QLabel(tr("就绪"), this);

    m_statusLayout->addWidget(m_totalNodesLabel);
    m_statusLayout->addWidget(m_visibleNodesLabel);
    m_statusLayout->addWidget(m_selectedNodesLabel);
    m_statusLayout->addWidget(m_loadProgressBar);
    m_statusLayout->addWidget(m_loadStatusLabel);

    // 添加到主布局
    m_mainLayout->addWidget(m_splitter);
    m_mainLayout->addWidget(m_controlGroup);
    m_mainLayout->addWidget(m_displayGroup);
    m_mainLayout->addWidget(m_statusGroup);

    // 设置分割器比例
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setSizes({300, 100});

    setWidget(m_mainWidget);
    setMinimumWidth(250);
    setMaximumWidth(400);
}

void ModelTreeDockWidget::setupControlPanel()
{
    m_controlGroup = new QGroupBox(tr("控制面板"), this);
    m_controlLayout = new QHBoxLayout(m_controlGroup);

    // 创建控制按钮
    m_showAllButton = new QPushButton(tr("全显"), this);
    m_hideAllButton = new QPushButton(tr("全隐"), this);
    m_expandAllButton = new QPushButton(tr("展开"), this);
    m_collapseAllButton = new QPushButton(tr("折叠"), this);
    m_refreshButton = new QPushButton(tr("刷新"), this);

    // 设置按钮大小
    QSize buttonSize(50, 25);
    m_showAllButton->setMaximumSize(buttonSize);
    m_hideAllButton->setMaximumSize(buttonSize);
    m_expandAllButton->setMaximumSize(buttonSize);
    m_collapseAllButton->setMaximumSize(buttonSize);
    m_refreshButton->setMaximumSize(buttonSize);

    // 添加到布局
    m_controlLayout->addWidget(m_showAllButton);
    m_controlLayout->addWidget(m_hideAllButton);
    m_controlLayout->addWidget(m_expandAllButton);
    m_controlLayout->addWidget(m_collapseAllButton);
    m_controlLayout->addWidget(m_refreshButton);

    // 连接信号
    connect(m_showAllButton, &QPushButton::clicked,
            this, &ModelTreeDockWidget::onShowAllClicked);
    connect(m_hideAllButton, &QPushButton::clicked,
            this, &ModelTreeDockWidget::onHideAllClicked);
    connect(m_expandAllButton, &QPushButton::clicked,
            this, &ModelTreeDockWidget::onExpandAllClicked);
    connect(m_collapseAllButton, &QPushButton::clicked,
            this, &ModelTreeDockWidget::onCollapseAllClicked);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &ModelTreeDockWidget::onRefreshClicked);
}

void ModelTreeDockWidget::setupDisplayOptions()
{
    m_displayGroup = new QGroupBox(tr("显示选项"), this);
    m_displayLayout = new QVBoxLayout(m_displayGroup);

    // 显示类型选择
    m_showAssembliesCheck = new QCheckBox(tr("显示装配体"), this);
    m_showPartsCheck = new QCheckBox(tr("显示零件"), this);
    m_showAssembliesCheck->setChecked(true);
    m_showPartsCheck->setChecked(true);

    // 透明度控制
    m_transparencyLabel = new QLabel(tr("透明度:"), this);
    m_transparencySlider = new QSlider(Qt::Horizontal, this);
    m_transparencySpin = new QSpinBox(this);
    
    m_transparencySlider->setRange(0, 100);
    m_transparencySlider->setValue(0);
    m_transparencySpin->setRange(0, 100);
    m_transparencySpin->setValue(0);
    m_transparencySpin->setSuffix("%");

    // 连接透明度控件
    connect(m_transparencySlider, &QSlider::valueChanged,
            m_transparencySpin, &QSpinBox::setValue);
    connect(m_transparencySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            m_transparencySlider, &QSlider::setValue);

    // 添加到布局
    m_displayLayout->addWidget(m_showAssembliesCheck);
    m_displayLayout->addWidget(m_showPartsCheck);
    m_displayLayout->addWidget(m_transparencyLabel);
    
    QHBoxLayout* transparencyLayout = new QHBoxLayout();
    transparencyLayout->addWidget(m_transparencySlider);
    transparencyLayout->addWidget(m_transparencySpin);
    m_displayLayout->addLayout(transparencyLayout);

    // 连接信号
    connect(m_showAssembliesCheck, &QCheckBox::toggled,
            this, &ModelTreeDockWidget::onShowAssembliesChanged);
    connect(m_showPartsCheck, &QCheckBox::toggled,
            this, &ModelTreeDockWidget::onShowPartsChanged);
    connect(m_transparencySlider, &QSlider::valueChanged,
            this, &ModelTreeDockWidget::onTransparencyChanged);
}

void ModelTreeDockWidget::loadSTEPFile(const QString& filePath)
{
    m_currentFilePath = filePath;
    m_isLoading = true;
    
    // 显示进度条
    m_loadProgressBar->setVisible(true);
    m_loadProgressBar->setValue(0);
    m_loadStatusLabel->setText(tr("正在加载..."));
    
    // 禁用控制按钮
    m_showAllButton->setEnabled(false);
    m_hideAllButton->setEnabled(false);
    m_expandAllButton->setEnabled(false);
    m_collapseAllButton->setEnabled(false);
    m_refreshButton->setEnabled(false);

    // 开始加载
    m_modelTreeWidget->loadSTEPFile(filePath);
}

std::vector<TopoDS_Shape> ModelTreeDockWidget::getVisibleShapes() const
{
    if (m_modelTreeWidget && m_modelTreeWidget->getModelTree()) {
        return m_modelTreeWidget->getModelTree()->getVisibleShapes();
    }
    return {};
}

void ModelTreeDockWidget::onModelTreeLoaded(bool success, const QString& message)
{
    m_isLoading = false;
    m_loadProgressBar->setVisible(false);
    
    if (success) {
        m_loadStatusLabel->setText(tr("加载完成"));
        updateStatistics();
        
        // 启用控制按钮
        m_showAllButton->setEnabled(true);
        m_hideAllButton->setEnabled(true);
        m_expandAllButton->setEnabled(true);
        m_collapseAllButton->setEnabled(true);
        m_refreshButton->setEnabled(true);
        
        // 发送初始可见形状
        emit modelVisibilityChanged(getVisibleShapes());
        
    } else {
        m_loadStatusLabel->setText(tr("加载失败"));
        QMessageBox::warning(this, tr("加载失败"), message);
    }
}

void ModelTreeDockWidget::onLoadProgress(int progress, const QString& message)
{
    m_loadProgressBar->setValue(progress);
    m_loadStatusLabel->setText(message);
}

void ModelTreeDockWidget::onVisibilityChanged(const std::vector<TopoDS_Shape>& visibleShapes)
{
    m_visibleNodes = static_cast<int>(visibleShapes.size());
    updateStatistics();
    
    emit modelVisibilityChanged(visibleShapes);
    emit renderUpdateRequested();
}

void ModelTreeDockWidget::onSelectionChanged(const std::vector<std::shared_ptr<STEPTreeNode>>& selectedNodes)
{
    m_selectedNodes = static_cast<int>(selectedNodes.size());
    updateStatistics();
    
    // 收集选中节点的形状
    std::vector<TopoDS_Shape> selectedShapes;
    for (const auto& node : selectedNodes) {
        if (!node->shape.IsNull()) {
            selectedShapes.push_back(node->shape);
        }
    }
    
    emit modelSelectionChanged(selectedShapes);
}

void ModelTreeDockWidget::onShowAllClicked()
{
    if (m_modelTreeWidget && m_modelTreeWidget->getModelTree()) {
        auto rootNode = m_modelTreeWidget->getModelTree()->getRootNode();
        if (rootNode) {
            m_modelTreeWidget->getModelTree()->setNodeVisibility(rootNode, true, true);
        }
    }
}

void ModelTreeDockWidget::onHideAllClicked()
{
    if (m_modelTreeWidget && m_modelTreeWidget->getModelTree()) {
        auto rootNode = m_modelTreeWidget->getModelTree()->getRootNode();
        if (rootNode) {
            m_modelTreeWidget->getModelTree()->setNodeVisibility(rootNode, false, true);
        }
    }
}

void ModelTreeDockWidget::onExpandAllClicked()
{
    if (m_modelTreeWidget) {
        m_modelTreeWidget->getTreeView()->expandAll();
    }
}

void ModelTreeDockWidget::onCollapseAllClicked()
{
    if (m_modelTreeWidget) {
        m_modelTreeWidget->getTreeView()->collapseAll();
    }
}

void ModelTreeDockWidget::onRefreshClicked()
{
    if (!m_currentFilePath.isEmpty()) {
        loadSTEPFile(m_currentFilePath);
    }
}

void ModelTreeDockWidget::onShowAssembliesChanged(bool show)
{
    Q_UNUSED(show)
    // TODO: 实现装配体显示/隐藏逻辑
    qDebug() << "Show assemblies changed:" << show;
}

void ModelTreeDockWidget::onShowPartsChanged(bool show)
{
    Q_UNUSED(show)
    // TODO: 实现零件显示/隐藏逻辑
    qDebug() << "Show parts changed:" << show;
}

void ModelTreeDockWidget::onTransparencyChanged(int value)
{
    Q_UNUSED(value)
    // TODO: 实现透明度变化逻辑
    qDebug() << "Transparency changed:" << value;
    emit renderUpdateRequested();
}

void ModelTreeDockWidget::updateStatistics()
{
    if (m_modelTreeWidget && m_modelTreeWidget->getModelTree()) {
        auto stats = m_modelTreeWidget->getModelTree()->getModelStats();
        m_totalNodes = stats.totalNodes;
        
        m_totalNodesLabel->setText(tr("总节点: %1").arg(m_totalNodes));
        m_visibleNodesLabel->setText(tr("可见节点: %1").arg(m_visibleNodes));
        m_selectedNodesLabel->setText(tr("选中节点: %1").arg(m_selectedNodes));
    }
}