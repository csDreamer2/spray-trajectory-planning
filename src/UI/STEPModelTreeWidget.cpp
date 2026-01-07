#include "STEPModelTreeWidget.h"

#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

// VTK includes (需要完整定义)
#include <vtkRenderer.h>

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
    
    connect(m_treeWidget, &QTreeWidget::itemClicked, 
            this, &STEPModelTreeWidget::onItemClicked);
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
    if (column == 1) {
        // 切换可见性 - 递归处理子节点
        bool visible = (item->checkState(1) == Qt::Checked);
        setItemVisibilityRecursive(item, visible);
        
        // 立即刷新渲染
        emit partVisibilityChanged(item->data(0, Qt::UserRole).toString(), visible);
        
    } else if (column == 0) {
        // 高亮选中的部件
        QString partName = item->data(0, Qt::UserRole).toString();
        
        // 重置所有部件颜色
        for (auto it = m_actorMap.begin(); it != m_actorMap.end(); ++it) {
            it.value()->GetProperty()->SetColor(0.8, 0.8, 0.9);
        }
        
        // 高亮选中部件及其所有子部件（橙色，参考样例）
        highlightItemRecursive(item);
        
        // 立即刷新渲染
        emit partVisibilityChanged(partName, true);
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
