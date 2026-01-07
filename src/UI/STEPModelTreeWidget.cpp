#include "STEPModelTreeWidget.h"

#include <QApplication>
#include <QCoreApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QRegularExpression>

// VTK includes (需要完整定义)
#include <vtkRenderer.h>

// VTK XML读写
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkAppendPolyData.h>

// OpenCASCADE STEP读取
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_ChildIterator.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

// OpenCASCADE到VTK转换
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>

// VTK
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkIdList.h>

STEPModelTreeWidget::STEPModelTreeWidget(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_treeWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_shapeCounter(0)
    , m_contextMenu(nullptr)
    , m_expandAction(nullptr)
    , m_collapseAction(nullptr)
{
    setupUI();
    setupContextMenu();
}

STEPModelTreeWidget::~STEPModelTreeWidget()
{
    qDebug() << "STEPModelTreeWidget: 析构";
    clearScene();
}

void STEPModelTreeWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // 状态标签
    m_statusLabel = new QLabel("准备加载STEP模型...", this);
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; padding: 4px; }");
    m_layout->addWidget(m_statusLabel);

    // 树形视图
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels(QStringList() << "部件名称" << "可见");
    m_treeWidget->setColumnWidth(0, 200);
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // 点击项目名称时高亮
    connect(m_treeWidget, &QTreeWidget::itemClicked, 
            this, &STEPModelTreeWidget::onItemClicked);
    // 勾选框状态变化时切换可见性
    connect(m_treeWidget, &QTreeWidget::itemChanged,
            this, &STEPModelTreeWidget::onItemChanged);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,
            this, &STEPModelTreeWidget::onContextMenuRequested);
    
    m_layout->addWidget(m_treeWidget);
}

void STEPModelTreeWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    m_expandAction = m_contextMenu->addAction(tr("展开所有"), [this]() {
        m_treeWidget->expandAll();
    });
    
    m_collapseAction = m_contextMenu->addAction(tr("折叠所有"), [this]() {
        m_treeWidget->collapseAll();
    });
}

bool STEPModelTreeWidget::loadSTEPFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::critical(this, "错误", "STEP文件不存在");
        return false;
    }
    
    qDebug() << "STEPModelTreeWidget: 开始同步加载STEP文件:" << filePath;
    m_statusLabel->setText("正在加载STEP文件...");
    QApplication::processEvents();
    
    clearScene();
    
    try {
        // 创建OCAF文档
        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        app->NewDocument("MDTV-XCAF", m_occDoc);
        
        // 读取STEP文件
        STEPCAFControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());
        
        if (status != IFSelect_RetDone) {
            QMessageBox::critical(this, "错误", "无法读取STEP文件");
            m_statusLabel->setText("加载失败");
            return false;
        }
        
        qDebug() << "STEPModelTreeWidget: STEP文件读取成功";
        m_statusLabel->setText("正在解析STEP数据...");
        QApplication::processEvents();
        
        if (!reader.Transfer(m_occDoc)) {
            QMessageBox::critical(this, "错误", "无法转换STEP数据");
            m_statusLabel->setText("加载失败");
            return false;
        }
        
        qDebug() << "STEPModelTreeWidget: STEP数据转换成功";
        m_statusLabel->setText("正在构建模型树...");
        QApplication::processEvents();
        
        // 获取形状工具
        Handle(XCAFDoc_ShapeTool) shapeTool = 
            XCAFDoc_DocumentTool::ShapeTool(m_occDoc->Main());
        
        // 遍历所有顶层形状
        TDF_LabelSequence labels;
        shapeTool->GetFreeShapes(labels);
        
        qDebug() << "STEPModelTreeWidget: 找到" << labels.Length() << "个顶层形状";
        
        for (Standard_Integer i = 1; i <= labels.Length(); i++) {
            TDF_Label label = labels.Value(i);
            TopoDS_Shape shape = shapeTool->GetShape(label);
            
            if (!shape.IsNull()) {
                processShape(shape, label, nullptr);
            }
            
            // 定期更新UI
            if (i % 10 == 0) {
                QApplication::processEvents();
            }
        }
        
        qDebug() << "STEPModelTreeWidget: 模型树构建完成";
        m_statusLabel->setText(QString("STEP模型加载成功 (%1个部件)").arg(m_shapeCounter));
        
        // 展开第一层
        m_treeWidget->expandToDepth(1);
        
        emit loadCompleted(true, "STEP模型加载成功");
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: 加载异常:" << e.what();
        QMessageBox::critical(this, "错误", QString("加载异常: %1").arg(e.what()));
        m_statusLabel->setText("加载失败");
        emit loadCompleted(false, QString("加载异常: %1").arg(e.what()));
        return false;
    } catch (...) {
        qCritical() << "STEPModelTreeWidget: 未知异常";
        QMessageBox::critical(this, "错误", "未知异常");
        m_statusLabel->setText("加载失败");
        emit loadCompleted(false, "未知异常");
        return false;
    }
}

void STEPModelTreeWidget::processShape(const TopoDS_Shape& shape, 
                                        const TDF_Label& label,
                                        QTreeWidgetItem* parentItem)
{
    // 获取形状名称
    Handle(TDataStd_Name) nameAttr;
    QString shapeName;
    if (label.FindAttribute(TDataStd_Name::GetID(), nameAttr)) {
        shapeName = QString::fromUtf8(TCollection_AsciiString(nameAttr->Get()).ToCString());
    } else {
        shapeName = QString("Part_%1").arg(++m_shapeCounter);
    }
    
    // 创建树节点
    QTreeWidgetItem* item = parentItem ? 
        new QTreeWidgetItem(parentItem) : new QTreeWidgetItem(m_treeWidget);
    item->setText(0, shapeName);
    item->setCheckState(1, Qt::Checked);
    item->setData(0, Qt::UserRole, shapeName);
    
    // 处理子形状
    Handle(XCAFDoc_ShapeTool) shapeTool = 
        XCAFDoc_DocumentTool::ShapeTool(m_occDoc->Main());
    
    TDF_LabelSequence components;
    bool hasComponents = shapeTool->GetComponents(label, components) && components.Length() > 0;
    
    if (hasComponents) {
        // 有子组件，递归处理子组件，不创建Actor
        for (Standard_Integer i = 1; i <= components.Length(); i++) {
            TDF_Label compLabel = components.Value(i);
            TopoDS_Shape compShape = shapeTool->GetShape(compLabel);
            if (!compShape.IsNull()) {
                processShape(compShape, compLabel, item);
            }
        }
    } else {
        // 叶子节点，创建VTK Actor
        vtkSmartPointer<vtkActor> actor = createActorFromShape(shape);
        if (actor) {
            m_actorMap[shapeName] = actor;
            m_shapeMap[shapeName] = shape;
            
            // NAUO8 默认不显示（机器人底座/安装板）
            if (shapeName == "NAUO8") {
                actor->SetVisibility(false);
                item->setCheckState(1, Qt::Unchecked);
                qDebug() << "STEPModelTreeWidget: NAUO8 默认隐藏";
            }
            
            qDebug() << "STEPModelTreeWidget: 创建Actor成功:" << shapeName;
        }
    }
}

vtkSmartPointer<vtkActor> STEPModelTreeWidget::createActorFromShape(const TopoDS_Shape& shape)
{
    // 网格化形状
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    
    // 遍历所有面
    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
        TopoDS_Face face = TopoDS::Face(exp.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, loc);
        
        if (triangulation.IsNull()) continue;
        
        gp_Trsf transform = loc.Transformation();
        Standard_Integer offset = points->GetNumberOfPoints();
        
        // 添加顶点 - OCC 7.7 API
        Standard_Integer nbNodes = triangulation->NbNodes();
        for (Standard_Integer i = 1; i <= nbNodes; i++) {
            gp_Pnt p = triangulation->Node(i).Transformed(transform);
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
        }
        
        // 添加三角形 - OCC 7.7 API
        Standard_Integer nbTriangles = triangulation->NbTriangles();
        for (Standard_Integer i = 1; i <= nbTriangles; i++) {
            const Poly_Triangle& tri = triangulation->Triangle(i);
            Standard_Integer n1, n2, n3;
            tri.Get(n1, n2, n3);
            
            // 使用vtkIdList
            vtkSmartPointer<vtkIdList> idList = vtkSmartPointer<vtkIdList>::New();
            idList->SetNumberOfIds(3);
            idList->SetId(0, offset + n1 - 1);
            idList->SetId(1, offset + n2 - 1);
            idList->SetId(2, offset + n3 - 1);
            triangles->InsertNextCell(idList);
        }
    }
    
    if (points->GetNumberOfPoints() == 0) {
        return nullptr;
    }
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(triangles);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.8, 0.8, 0.9);
    actor->GetProperty()->SetSpecular(0.3);
    actor->GetProperty()->SetSpecularPower(20);
    
    return actor;
}

void STEPModelTreeWidget::onItemClicked(QTreeWidgetItem* item, int column)
{
    if (column == 0) {
        // 点击部件名称时高亮选中的部件
        QString partName = item->data(0, Qt::UserRole).toString();
        
        qDebug() << "STEPModelTreeWidget: 点击部件:" << partName;
        
        // 重置所有部件颜色（只修改属性，不刷新渲染）
        for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
            it.value()->GetProperty()->SetColor(0.8, 0.8, 0.9);
        }
        
        // 高亮选中部件及其所有子部件（橙色）
        highlightItemRecursive(item);
        
        // 发送信号刷新渲染（只发送一次，不在循环中）
        emit partVisibilityChanged(partName, true);
    }
}

void STEPModelTreeWidget::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column == 1) {
        // 勾选框状态变化 - 切换可见性
        bool visible = (item->checkState(1) == Qt::Checked);
        QString partName = item->data(0, Qt::UserRole).toString();
        
        qDebug() << "STEPModelTreeWidget: 可见性变化:" << partName << visible;
        
        // 设置当前节点的可见性
        if (m_actorMap.contains(partName)) {
            m_actorMap[partName]->SetVisibility(visible);
        }
        
        // 阻止信号递归
        m_treeWidget->blockSignals(true);
        
        // 递归设置子节点
        for (int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem* child = item->child(i);
            child->setCheckState(1, visible ? Qt::Checked : Qt::Unchecked);
            setItemVisibilityRecursive(child, visible);
        }
        
        m_treeWidget->blockSignals(false);
        
        // 发送信号刷新渲染
        emit partVisibilityChanged(partName, visible);
    }
}

void STEPModelTreeWidget::setItemVisibilityRecursive(QTreeWidgetItem* item, bool visible)
{
    if (!item) return;
    
    // 设置当前节点的可见性
    QString partName = item->data(0, Qt::UserRole).toString();
    if (m_actorMap.contains(partName)) {
        m_actorMap[partName]->SetVisibility(visible);
        qDebug() << "STEPModelTreeWidget: 设置部件可见性:" << partName << visible;
    }
    
    // 递归设置所有子节点
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* child = item->child(i);
        child->setCheckState(1, visible ? Qt::Checked : Qt::Unchecked);
        setItemVisibilityRecursive(child, visible);
    }
}

void STEPModelTreeWidget::highlightItemRecursive(QTreeWidgetItem* item)
{
    if (!item) return;
    
    // 高亮当前节点
    QString partName = item->data(0, Qt::UserRole).toString();
    if (m_actorMap.contains(partName)) {
        m_actorMap[partName]->GetProperty()->SetColor(1.0, 0.5, 0.0);  // 橙色
        qDebug() << "STEPModelTreeWidget: 高亮部件:" << partName;
    }
    
    // 递归高亮所有子节点
    for (int i = 0; i < item->childCount(); ++i) {
        highlightItemRecursive(item->child(i));
    }
}

void STEPModelTreeWidget::onContextMenuRequested(const QPoint& pos)
{
    auto index = m_treeWidget->indexAt(pos);
    if (index.isValid()) {
        m_contextMenu->exec(m_treeWidget->mapToGlobal(pos));
    }
}

void STEPModelTreeWidget::clearScene()
{
    m_actorMap.clear();
    m_shapeMap.clear();
    m_treeWidget->clear();
    m_shapeCounter = 0;
    
    if (!m_occDoc.IsNull()) {
        m_occDoc.Nullify();
    }
    
    qDebug() << "STEPModelTreeWidget: 场景已清空";
}

void STEPModelTreeWidget::addActorsToRenderer(vtkRenderer* renderer)
{
    if (!renderer) {
        qWarning() << "STEPModelTreeWidget: renderer为空";
        return;
    }
    
    for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
        renderer->AddActor(it.value());
    }
    
    qDebug() << "STEPModelTreeWidget: 添加了" << m_actorMap.size() << "个Actor到渲染器";
}

void STEPModelTreeWidget::removeActorsFromRenderer(vtkRenderer* renderer)
{
    if (!renderer) {
        qWarning() << "STEPModelTreeWidget: renderer为空";
        return;
    }
    
    for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
        renderer->RemoveActor(it.value());
    }
    
    qDebug() << "STEPModelTreeWidget: 从渲染器移除了" << m_actorMap.size() << "个Actor";
}

void STEPModelTreeWidget::applyTransformToAllActors(vtkTransform* transform)
{
    if (!transform) {
        qWarning() << "STEPModelTreeWidget: transform为空";
        return;
    }
    
    // 应用变换到所有可见的Actor
    for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
        vtkActor* actor = it.value();
        if (actor && actor->GetVisibility()) {
            // 使用C风格强制转换
            actor->SetUserTransform((vtkLinearTransform*)transform);
        }
    }
    
    qDebug() << "STEPModelTreeWidget: 应用变换到" << m_actorMap.size() << "个Actor";
}

void STEPModelTreeWidget::applyTransformToActor(const QString& partName, vtkTransform* transform)
{
    if (!transform) {
        qWarning() << "STEPModelTreeWidget: transform为空";
        return;
    }
    
    if (!m_actorMap.contains(partName)) {
        qWarning() << "STEPModelTreeWidget: 找不到部件:" << partName;
        return;
    }
    
    vtkActor* actor = m_actorMap[partName];
    if (actor) {
        // 使用C风格强制转换
        actor->SetUserTransform((vtkLinearTransform*)transform);
        qDebug() << "STEPModelTreeWidget: 应用变换到部件:" << partName;
    }
}

void STEPModelTreeWidget::setPartVisibility(const QString& partName, bool visible)
{
    if (!m_actorMap.contains(partName)) {
        qWarning() << "STEPModelTreeWidget: 找不到部件:" << partName;
        return;
    }
    
    vtkActor* actor = m_actorMap[partName];
    if (actor) {
        actor->SetVisibility(visible);
        qDebug() << "STEPModelTreeWidget: 设置部件可见性:" << partName << visible;
    }
}

// ==================== 缓存功能实现 ====================

QString STEPModelTreeWidget::getCachePath(const QString& stepFilePath)
{
    QFileInfo fileInfo(stepFilePath);
    QString baseName = fileInfo.completeBaseName();
    
    // 使用文件修改时间生成hash，确保文件更新后缓存失效
    QDateTime modTime = fileInfo.lastModified();
    QString timeStr = modTime.toString(Qt::ISODate);
    QByteArray hash = QCryptographicHash::hash(timeStr.toUtf8(), QCryptographicHash::Md5);
    QString hashStr = hash.toHex().left(8);
    
    // 获取项目根目录（向上3级：Debug -> bin -> build -> 项目根）
    QDir appDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();  // bin
    appDir.cdUp();  // build
    appDir.cdUp();  // 项目根
    
    QString projectRoot = appDir.absolutePath();
    QString cacheDir = projectRoot + "/data/cache";
    
    // 确保缓存目录存在
    QDir dir;
    if (!dir.exists(cacheDir)) {
        dir.mkpath(cacheDir);
        qDebug() << "STEPModelTreeWidget: 创建缓存目录:" << cacheDir;
    }
    
    QString cachePath = QString("%1/%2_%3.vtp").arg(cacheDir, baseName, hashStr);
    qDebug() << "STEPModelTreeWidget: 缓存路径:" << cachePath;
    
    return cachePath;
}

bool STEPModelTreeWidget::isCacheValid(const QString& cachePath, const QString& stepFilePath)
{
    // 检查JSON文件是否存在（树结构）
    QString jsonPath = cachePath;
    jsonPath.replace(".vtp", ".json");
    QFileInfo jsonInfo(jsonPath);
    
    if (!jsonInfo.exists()) {
        qDebug() << "STEPModelTreeWidget: JSON缓存文件不存在:" << jsonPath;
        return false;
    }
    
    // 检查缓存是否比STEP文件新
    QFileInfo stepInfo(stepFilePath);
    if (jsonInfo.lastModified() < stepInfo.lastModified()) {
        qDebug() << "STEPModelTreeWidget: 缓存过期（STEP文件更新）";
        return false;
    }
    
    qDebug() << "STEPModelTreeWidget: 缓存有效";
    return true;
}

bool STEPModelTreeWidget::saveToCache(const QString& cachePath)
{
    try {
        qDebug() << "STEPModelTreeWidget: 开始保存缓存到:" << cachePath;
        m_statusLabel->setText("正在保存缓存...");
        QApplication::processEvents();
        
        if (m_actorMap.isEmpty()) {
            qWarning() << "STEPModelTreeWidget: 没有可保存的数据";
            return false;
        }
        
        // 1. 保存树结构到JSON文件
        QString jsonPath = cachePath;
        jsonPath.replace(".vtp", ".json");
        if (!saveTreeStructure(jsonPath)) {
            qWarning() << "STEPModelTreeWidget: 树结构保存失败";
            // 继续保存VTP，即使JSON失败
        }
        
        // 2. 为每个部件保存独立的VTP文件
        QString cacheDir = QFileInfo(cachePath).absolutePath();
        QString baseName = QFileInfo(cachePath).completeBaseName();
        
        int savedCount = 0;
        for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
            QString partName = it.key();
            vtkActor* actor = it.value();
            
            vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
            if (!mapper || !mapper->GetInput()) {
                continue;
            }
            
            // 为每个部件创建独立的VTP文件
            // 使用安全的文件名（移除特殊字符）
            QString safePartName = partName;
            safePartName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
            QString partCachePath = QString("%1/%2_%3.vtp").arg(cacheDir, baseName, safePartName);
            
            vtkSmartPointer<vtkXMLPolyDataWriter> writer = 
                vtkSmartPointer<vtkXMLPolyDataWriter>::New();
            writer->SetFileName(partCachePath.toStdString().c_str());
            writer->SetInputData(mapper->GetInput());
            writer->SetDataModeToBinary();
            writer->SetCompressorTypeToNone();
            
            if (writer->Write()) {
                savedCount++;
            } else {
                qWarning() << "STEPModelTreeWidget: 部件保存失败:" << partName;
            }
            
            // 定期更新UI
            if (savedCount % 10 == 0) {
                m_statusLabel->setText(QString("正在保存缓存... (%1/%2)").arg(savedCount).arg(m_actorMap.size()));
                QApplication::processEvents();
            }
        }
        
        if (savedCount == 0) {
            qCritical() << "STEPModelTreeWidget: 没有部件保存成功";
            return false;
        }
        
        qDebug() << "STEPModelTreeWidget: 缓存保存成功，保存了" << savedCount << "个部件";
        m_statusLabel->setText(QString("缓存保存成功 (%1个部件)").arg(savedCount));
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: 保存缓存失败:" << e.what();
        m_statusLabel->setText("缓存保存失败");
        return false;
    } catch (...) {
        qCritical() << "STEPModelTreeWidget: 保存缓存失败（未知异常）";
        m_statusLabel->setText("缓存保存失败");
        return false;
    }
}

bool STEPModelTreeWidget::loadFromCache(const QString& cachePath)
{
    try {
        qDebug() << "STEPModelTreeWidget: 从缓存加载:" << cachePath;
        m_statusLabel->setText("正在从缓存加载...");
        QApplication::processEvents();
        
        // 1. 加载树结构
        QString jsonPath = cachePath;
        jsonPath.replace(".vtp", ".json");
        qDebug() << "STEPModelTreeWidget: JSON路径:" << jsonPath;
        bool treeLoaded = loadTreeStructure(jsonPath);
        
        if (!treeLoaded) {
            qWarning() << "STEPModelTreeWidget: 树结构加载失败";
            return false;
        }
        
        qDebug() << "STEPModelTreeWidget: 树结构加载成功";
        
        // 2. 为每个部件加载独立的VTP文件
        QString cacheDir = QFileInfo(cachePath).absolutePath();
        QString baseName = QFileInfo(cachePath).completeBaseName();
        
        qDebug() << "STEPModelTreeWidget: 缓存目录:" << cacheDir;
        qDebug() << "STEPModelTreeWidget: 基础名称:" << baseName;
        
        int loadedCount = 0;
        int totalParts = 0;
        
        // 遍历树，找到所有叶子节点
        QTreeWidgetItemIterator it(m_treeWidget);
        while (*it) {
            QTreeWidgetItem* item = *it;
            QString partName = item->data(0, Qt::UserRole).toString();
            
            // 只处理叶子节点（没有子节点的节点）
            if (!partName.isEmpty() && item->childCount() == 0) {
                totalParts++;
                
                // 构建部件缓存文件路径
                QString safePartName = partName;
                safePartName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
                QString partCachePath = QString("%1/%2_%3.vtp").arg(cacheDir, baseName, safePartName);
                
                qDebug() << "STEPModelTreeWidget: 尝试加载部件:" << partName << "-> 文件:" << partCachePath;
                
                QFileInfo partFileInfo(partCachePath);
                if (!partFileInfo.exists()) {
                    qWarning() << "STEPModelTreeWidget: 部件缓存文件不存在:" << partCachePath;
                    ++it;
                    continue;
                }
                
                // 读取部件VTP文件
                vtkSmartPointer<vtkXMLPolyDataReader> reader = 
                    vtkSmartPointer<vtkXMLPolyDataReader>::New();
                reader->SetFileName(partCachePath.toStdString().c_str());
                reader->Update();
                
                vtkPolyData* polyData = reader->GetOutput();
                if (!polyData || polyData->GetNumberOfPoints() == 0) {
                    qWarning() << "STEPModelTreeWidget: 部件缓存文件无效:" << partName;
                    ++it;
                    continue;
                }
                
                // 创建独立的Actor
                vtkSmartPointer<vtkPolyDataMapper> mapper = 
                    vtkSmartPointer<vtkPolyDataMapper>::New();
                mapper->SetInputData(polyData);
                
                vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
                actor->SetMapper(mapper);
                actor->GetProperty()->SetColor(0.8, 0.8, 0.9);
                actor->GetProperty()->SetSpecular(0.3);
                actor->GetProperty()->SetSpecularPower(20);
                
                // 保存到Actor映射
                m_actorMap[partName] = actor;
                loadedCount++;
                
                // NAUO8 默认不显示（机器人底座/安装板）
                if (partName == "NAUO8") {
                    actor->SetVisibility(false);
                    item->setCheckState(1, Qt::Unchecked);  // 第1列是可见性勾选框
                    qDebug() << "STEPModelTreeWidget: NAUO8 默认隐藏";
                }
                
                // 定期更新UI
                if (loadedCount % 10 == 0) {
                    m_statusLabel->setText(QString("正在加载缓存... (%1/%2)").arg(loadedCount).arg(totalParts));
                    QApplication::processEvents();
                }
            }
            
            ++it;
        }
        
        if (loadedCount == 0) {
            qWarning() << "STEPModelTreeWidget: 没有加载到任何部件";
            return false;
        }
        
        qDebug() << "STEPModelTreeWidget: 从缓存加载成功，加载了" << loadedCount << "个部件";
        m_statusLabel->setText(QString("从缓存加载成功 (%1个部件)").arg(loadedCount));
        
        // 展开第一层
        m_treeWidget->expandToDepth(1);
        
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: 从缓存加载失败:" << e.what();
        return false;
    } catch (...) {
        qCritical() << "STEPModelTreeWidget: 从缓存加载失败（未知异常）";
        return false;
    }
}

bool STEPModelTreeWidget::loadSTEPFileFast(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::critical(this, "错误", "STEP文件不存在");
        return false;
    }
    
    clearScene();
    
    // 获取缓存路径
    QString cachePath = getCachePath(filePath);
    
    // 检查缓存是否有效
    if (isCacheValid(cachePath, filePath)) {
        qDebug() << "STEPModelTreeWidget: 使用缓存快速加载";
        
        if (loadFromCache(cachePath)) {
            emit loadCompleted(true, "从缓存快速加载成功");
            return true;
        } else {
            qWarning() << "STEPModelTreeWidget: 缓存加载失败，回退到正常加载";
        }
    } else {
        qDebug() << "STEPModelTreeWidget: 缓存不存在或已过期，执行正常加载";
    }
    
    // 缓存无效或加载失败，执行正常STEP加载
    bool success = loadSTEPFile(filePath);
    
    // 如果加载成功，保存缓存
    if (success) {
        qDebug() << "STEPModelTreeWidget: 正常加载成功，保存缓存";
        saveToCache(cachePath);
    }
    
    return success;
}

// ==================== 树结构保存/加载 ====================

QJsonObject STEPModelTreeWidget::treeItemToJson(QTreeWidgetItem* item)
{
    if (!item) return QJsonObject();
    
    QJsonObject obj;
    obj["name"] = item->text(0);
    obj["checked"] = (item->checkState(1) == Qt::Checked);
    obj["partName"] = item->data(0, Qt::UserRole).toString();
    
    // 递归保存子节点
    if (item->childCount() > 0) {
        QJsonArray children;
        for (int i = 0; i < item->childCount(); ++i) {
            children.append(treeItemToJson(item->child(i)));
        }
        obj["children"] = children;
    }
    
    return obj;
}

QTreeWidgetItem* STEPModelTreeWidget::jsonToTreeItem(const QJsonObject& json, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = parent ? 
        new QTreeWidgetItem(parent) : new QTreeWidgetItem(m_treeWidget);
    
    item->setText(0, json["name"].toString());
    item->setCheckState(1, json["checked"].toBool() ? Qt::Checked : Qt::Unchecked);
    item->setData(0, Qt::UserRole, json["partName"].toString());
    
    // 递归加载子节点
    if (json.contains("children")) {
        QJsonArray children = json["children"].toArray();
        for (const QJsonValue& childValue : children) {
            jsonToTreeItem(childValue.toObject(), item);
        }
    }
    
    return item;
}

bool STEPModelTreeWidget::saveTreeStructure(const QString& jsonPath)
{
    try {
        QJsonArray rootArray;
        
        // 保存所有顶层节点
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
            rootArray.append(treeItemToJson(item));
        }
        
        QJsonObject root;
        root["version"] = "1.0";
        root["shapeCounter"] = m_shapeCounter;
        root["tree"] = rootArray;
        
        // 写入JSON文件
        QFile file(jsonPath);
        if (!file.open(QIODevice::WriteOnly)) {
            qCritical() << "STEPModelTreeWidget: 无法创建JSON文件:" << jsonPath;
            return false;
        }
        
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Compact));
        file.close();
        
        qDebug() << "STEPModelTreeWidget: 树结构已保存到:" << jsonPath;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: 保存树结构失败:" << e.what();
        return false;
    }
}

bool STEPModelTreeWidget::loadTreeStructure(const QString& jsonPath)
{
    try {
        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "STEPModelTreeWidget: 无法打开JSON文件:" << jsonPath;
            return false;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "STEPModelTreeWidget: JSON格式无效";
            return false;
        }
        
        QJsonObject root = doc.object();
        m_shapeCounter = root["shapeCounter"].toInt();
        
        QJsonArray rootArray = root["tree"].toArray();
        for (const QJsonValue& value : rootArray) {
            jsonToTreeItem(value.toObject(), nullptr);
        }
        
        qDebug() << "STEPModelTreeWidget: 树结构已加载，共" << m_shapeCounter << "个节点";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "STEPModelTreeWidget: 加载树结构失败:" << e.what();
        return false;
    }
}
