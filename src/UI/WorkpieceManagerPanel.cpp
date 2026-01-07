#include "WorkpieceManagerPanel.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QCoreApplication>
#include <QProcess>

namespace UI {

WorkpieceManagerPanel::WorkpieceManagerPanel(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_countLabel(nullptr)
    , m_workpieceList(nullptr)
    , m_addButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
    , m_contextMenu(nullptr)
{
    setupUI();
    setupConnections();
    loadWorkpieceList();
}

WorkpieceManagerPanel::~WorkpieceManagerPanel()
{
}

void WorkpieceManagerPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // 标题和统计
    m_titleLabel = new QLabel("工件管理", this);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   color: #ffffff;"
        "   background-color: #2b2b2b;"
        "   padding: 8px;"
        "   border-radius: 4px;"
        "}"
    );

    m_countLabel = new QLabel("工件数量: 0", this);
    m_countLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: #cccccc;"
        "   background-color: #2b2b2b;"
        "   padding: 5px;"
        "   border-radius: 3px;"
        "}"
    );

    // 工件列表 - 黑底白字
    m_workpieceList = new QListWidget(this);
    m_workpieceList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_workpieceList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_workpieceList->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #555555;"
        "   border-radius: 4px;"
        "   background-color: #1e1e1e;"
        "   color: #ffffff;"
        "   font-size: 13px;"
        "   padding: 4px;"
        "}"
        "QListWidget::item {"
        "   padding: 10px;"
        "   border-bottom: 1px solid #333333;"
        "   color: #ffffff;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #2d2d2d;"
        "   color: #ffffff;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #0d47a1;"
        "   color: #ffffff;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: #2b2b2b;"
        "   width: 12px;"
        "   border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #555555;"
        "   border-radius: 6px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #666666;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    );

    // 按钮区域
    m_buttonLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("添加工件", this);
    m_addButton->setIcon(QIcon::fromTheme("list-add"));
    m_addButton->setToolTip("从文件系统添加点云文件");
    
    m_deleteButton = new QPushButton("删除工件", this);
    m_deleteButton->setIcon(QIcon::fromTheme("list-remove"));
    m_deleteButton->setToolTip("删除选中的工件文件");
    m_deleteButton->setEnabled(false);
    
    m_refreshButton = new QPushButton("刷新列表", this);
    m_refreshButton->setIcon(QIcon::fromTheme("view-refresh"));
    m_refreshButton->setToolTip("重新扫描工件目录");

    // 按钮样式 - 黑底白字
    QString buttonStyle = 
        "QPushButton {"
        "   padding: 8px 16px;"
        "   border: 1px solid #555555;"
        "   border-radius: 4px;"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3d3d3d;"
        "   border-color: #777777;"
        "   color: #ffffff;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e1e1e;"
        "   border-color: #888888;"
        "   color: #ffffff;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #1a1a1a;"
        "   color: #666666;"
        "   border-color: #333333;"
        "}";
    
    m_addButton->setStyleSheet(buttonStyle);
    m_deleteButton->setStyleSheet(buttonStyle);
    m_refreshButton->setStyleSheet(buttonStyle);

    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addWidget(m_refreshButton);
    m_buttonLayout->addStretch();

    // 右键菜单
    m_contextMenu = new QMenu(this);
    m_contextMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #2b2b2b;"
        "   color: #ffffff;"
        "   border: 1px solid #555555;"
        "   padding: 4px;"
        "}"
        "QMenu::item {"
        "   padding: 6px 20px;"
        "   background-color: transparent;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #0d47a1;"
        "   color: #ffffff;"
        "}"
        "QMenu::separator {"
        "   height: 1px;"
        "   background-color: #555555;"
        "   margin: 4px 0px;"
        "}"
    );
    
    m_visualizeAction = m_contextMenu->addAction("可视化");
    m_contextMenu->addSeparator();
    m_renameAction = m_contextMenu->addAction("重命名");
    m_showInExplorerAction = m_contextMenu->addAction("在文件管理器中显示");
    m_contextMenu->addSeparator();
    m_deleteAction = m_contextMenu->addAction("删除");

    // 布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_countLabel);
    m_mainLayout->addWidget(m_workpieceList, 1);
    m_mainLayout->addLayout(m_buttonLayout);

    // 设置整个面板的背景色
    setStyleSheet(
        "WorkpieceManagerPanel {"
        "   background-color: #1e1e1e;"
        "}"
    );

    setLayout(m_mainLayout);
}

void WorkpieceManagerPanel::setupConnections()
{
    connect(m_addButton, &QPushButton::clicked, this, &WorkpieceManagerPanel::onAddWorkpiece);
    connect(m_deleteButton, &QPushButton::clicked, this, &WorkpieceManagerPanel::onDeleteWorkpiece);
    connect(m_refreshButton, &QPushButton::clicked, this, &WorkpieceManagerPanel::onRefreshList);
    
    connect(m_workpieceList, &QListWidget::itemDoubleClicked, 
            this, &WorkpieceManagerPanel::onItemDoubleClicked);
    connect(m_workpieceList, &QListWidget::itemSelectionChanged,
            this, &WorkpieceManagerPanel::onItemSelectionChanged);
    connect(m_workpieceList, &QListWidget::customContextMenuRequested,
            this, &WorkpieceManagerPanel::onContextMenuRequested);
    
    // 右键菜单动作
    connect(m_visualizeAction, &QAction::triggered, this, [this]() {
        QListWidgetItem* item = m_workpieceList->currentItem();
        if (item) {
            onItemDoubleClicked(item);
        }
    });
    
    connect(m_deleteAction, &QAction::triggered, this, &WorkpieceManagerPanel::onDeleteWorkpiece);
    connect(m_renameAction, &QAction::triggered, this, &WorkpieceManagerPanel::onRenameWorkpiece);
    connect(m_showInExplorerAction, &QAction::triggered, this, &WorkpieceManagerPanel::onShowInExplorer);
}

void WorkpieceManagerPanel::loadWorkpieceList()
{
    m_workpieceList->clear();
    
    QString workpieceDir = getWorkpieceDirectory();
    QDir dir(workpieceDir);
    
    if (!dir.exists()) {
        qWarning() << "工件目录不存在:" << workpieceDir;
        m_countLabel->setText("工件数量: 0 (目录不存在)");
        return;
    }

    // 支持的点云文件格式
    QStringList filters;
    filters << "*.ply" << "*.PLY"
            << "*.pcd" << "*.PCD"
            << "*.stl" << "*.STL"
            << "*.obj" << "*.OBJ"
            << "*.asc" << "*.ASC";
    
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    
    for (const QFileInfo& fileInfo : fileList) {
        QString fileName = fileInfo.fileName();
        QString filePath = fileInfo.absoluteFilePath();
        qint64 fileSize = fileInfo.size();
        
        // 创建列表项
        QListWidgetItem* item = new QListWidgetItem(m_workpieceList);
        
        // 显示文件名和大小
        QString displayText = QString("%1 (%2)")
            .arg(fileName)
            .arg(formatFileSize(fileSize));
        item->setText(displayText);
        
        // 存储完整路径
        item->setData(Qt::UserRole, filePath);
        
        // 设置图标
        item->setIcon(getFileIcon(fileInfo.suffix().toLower()));
        
        // 添加工具提示
        QString tooltip = QString("文件: %1\n大小: %2\n修改时间: %3")
            .arg(fileName)
            .arg(formatFileSize(fileSize))
            .arg(fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        item->setToolTip(tooltip);
    }
    
    int count = m_workpieceList->count();
    m_countLabel->setText(QString("工件数量: %1").arg(count));
    
    qDebug() << "加载了" << count << "个工件文件";
    
    emit workpieceListUpdated(count);
}

QString WorkpieceManagerPanel::getWorkpieceDirectory() const
{
    // 获取项目根目录（向上3级：Debug -> bin -> build -> 项目根）
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();  // bin
    appDir.cdUp();  // build
    appDir.cdUp();  // 项目根
    
    QString projectRoot = appDir.absolutePath();
    QString workpieceDir = projectRoot + "/data/pointclouds";
    
    return workpieceDir;
}

QString WorkpieceManagerPanel::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = 1024 * KB;
    const qint64 GB = 1024 * MB;
    
    if (bytes >= GB) {
        return QString::number(bytes / (double)GB, 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / (double)MB, 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / (double)KB, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}

QIcon WorkpieceManagerPanel::getFileIcon(const QString& extension) const
{
    // 根据文件扩展名返回不同的图标
    if (extension == "ply") {
        return QIcon::fromTheme("application-x-object");
    } else if (extension == "pcd") {
        return QIcon::fromTheme("application-x-object");
    } else if (extension == "stl") {
        return QIcon::fromTheme("application-x-object");
    } else if (extension == "obj") {
        return QIcon::fromTheme("application-x-object");
    } else {
        return QIcon::fromTheme("text-x-generic");
    }
}

void WorkpieceManagerPanel::refreshWorkpieceList()
{
    loadWorkpieceList();
}

QString WorkpieceManagerPanel::getSelectedWorkpiecePath() const
{
    QListWidgetItem* item = m_workpieceList->currentItem();
    if (item) {
        return item->data(Qt::UserRole).toString();
    }
    return QString();
}

void WorkpieceManagerPanel::onAddWorkpiece()
{
    QString workpieceDir = getWorkpieceDirectory();
    
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择点云文件",
        workpieceDir,
        "点云文件 (*.ply *.PLY *.pcd *.PCD *.stl *.STL *.obj *.OBJ *.asc *.ASC);;所有文件 (*.*)"
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    QString targetPath = workpieceDir + "/" + fileInfo.fileName();
    
    // 如果文件已经在目标目录，不需要复制
    if (fileInfo.absoluteFilePath() == QFileInfo(targetPath).absoluteFilePath()) {
        QMessageBox::information(this, "提示", "文件已在工件目录中");
        refreshWorkpieceList();
        return;
    }
    
    // 检查是否已存在同名文件
    if (QFile::exists(targetPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "确认覆盖",
            QString("文件 %1 已存在，是否覆盖？").arg(fileInfo.fileName()),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            return;
        }
        
        QFile::remove(targetPath);
    }
    
    // 复制文件到工件目录
    if (QFile::copy(filePath, targetPath)) {
        qDebug() << "工件文件已添加:" << targetPath;
        QMessageBox::information(this, "成功", "工件文件已添加");
        refreshWorkpieceList();
    } else {
        qCritical() << "复制文件失败:" << filePath << "->" << targetPath;
        QMessageBox::critical(this, "错误", "复制文件失败");
    }
}

void WorkpieceManagerPanel::onDeleteWorkpiece()
{
    QListWidgetItem* item = m_workpieceList->currentItem();
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    QFileInfo fileInfo(filePath);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除工件文件吗？\n\n%1\n\n此操作不可恢复！").arg(fileInfo.fileName()),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (QFile::remove(filePath)) {
            qDebug() << "工件文件已删除:" << filePath;
            QMessageBox::information(this, "成功", "工件文件已删除");
            refreshWorkpieceList();
        } else {
            qCritical() << "删除文件失败:" << filePath;
            QMessageBox::critical(this, "错误", "删除文件失败");
        }
    }
}

void WorkpieceManagerPanel::onRefreshList()
{
    refreshWorkpieceList();
}

void WorkpieceManagerPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    
    QString filePath = item->data(Qt::UserRole).toString();
    qDebug() << "双击工件，请求可视化:" << filePath;
    
    emit workpieceDoubleClicked(filePath);
}

void WorkpieceManagerPanel::onItemSelectionChanged()
{
    bool hasSelection = m_workpieceList->currentItem() != nullptr;
    m_deleteButton->setEnabled(hasSelection);
    
    if (hasSelection) {
        QString filePath = getSelectedWorkpiecePath();
        emit workpieceSelected(filePath);
    }
}

void WorkpieceManagerPanel::onContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* item = m_workpieceList->itemAt(pos);
    if (item) {
        m_contextMenu->exec(m_workpieceList->mapToGlobal(pos));
    }
}

void WorkpieceManagerPanel::onShowInExplorer()
{
    QString filePath = getSelectedWorkpiecePath();
    if (filePath.isEmpty()) {
        return;
    }
    
    // 在文件管理器中显示文件
#ifdef Q_OS_WIN
    QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(filePath));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
#endif
}

void WorkpieceManagerPanel::onRenameWorkpiece()
{
    QListWidgetItem* item = m_workpieceList->currentItem();
    if (!item) {
        return;
    }
    
    QString oldPath = item->data(Qt::UserRole).toString();
    QFileInfo fileInfo(oldPath);
    
    bool ok;
    QString newName = QInputDialog::getText(
        this,
        "重命名工件",
        "请输入新的文件名:",
        QLineEdit::Normal,
        fileInfo.completeBaseName(),
        &ok
    );
    
    if (!ok || newName.isEmpty()) {
        return;
    }
    
    // 添加原始扩展名
    QString newFileName = newName + "." + fileInfo.suffix();
    QString newPath = fileInfo.absolutePath() + "/" + newFileName;
    
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "错误", "文件名已存在");
        return;
    }
    
    if (QFile::rename(oldPath, newPath)) {
        qDebug() << "工件文件已重命名:" << oldPath << "->" << newPath;
        refreshWorkpieceList();
    } else {
        qCritical() << "重命名文件失败:" << oldPath << "->" << newPath;
        QMessageBox::critical(this, "错误", "重命名文件失败");
    }
}

} // namespace UI
