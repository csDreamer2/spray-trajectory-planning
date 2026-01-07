#include "STEPLoadWorker.h"

#include <QDebug>
#include <QApplication>

// 注册自定义类型以便在信号中使用
typedef QMap<QString, vtkSmartPointer<vtkActor>> ActorMap;
typedef QMap<QString, TopoDS_Shape> ShapeMap;
Q_DECLARE_METATYPE(ActorMap)
Q_DECLARE_METATYPE(ShapeMap)

// OpenCASCADE STEP读取
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDataStd_Name.hxx>
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
#include <vtkAppendPolyData.h>

STEPLoadWorker::STEPLoadWorker(QObject* parent)
    : QObject(parent)
{
}

STEPLoadWorker::~STEPLoadWorker()
{
    qDebug() << "STEPLoadWorker: 析构";
}

void STEPLoadWorker::loadSTEPFile(const QString& filePath)
{
    qDebug() << "STEPLoadWorker: 开始加载STEP文件:" << filePath;
    
    try {
        // 创建OCAF文档
        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        app->NewDocument("MDTV-XCAF", m_occDoc);
        
        // 读取STEP文件
        STEPCAFControl_Reader reader;
        emit progressUpdated(10, 100, "正在读取STEP文件...");
        qDebug() << "STEPLoadWorker: 开始读取STEP文件...";
        
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());
        
        if (status != IFSelect_RetDone) {
            qCritical() << "STEPLoadWorker: 无法读取STEP文件，状态码=" << status;
            emit loadFinished(false, "无法读取STEP文件", m_actorMap, m_shapeMap, 0, "");
            return;
        }
        
        qDebug() << "STEPLoadWorker: STEP文件读取成功";
        emit progressUpdated(30, 100, "正在解析STEP数据...");
        
        qDebug() << "STEPLoadWorker: 开始转换STEP数据...";
        
        // Transfer可能会卡住，但我们无法中断它
        // 只能让它继续执行，并定期发送进度信号
        bool transferSuccess = reader.Transfer(m_occDoc);
        
        if (!transferSuccess) {
            qCritical() << "STEPLoadWorker: 无法转换STEP数据";
            emit loadFinished(false, "无法转换STEP数据", m_actorMap, m_shapeMap, 0, "");
            return;
        }
        
        qDebug() << "STEPLoadWorker: STEP数据转换成功";
        emit progressUpdated(50, 100, "正在构建模型树...");
        
        // 获取形状工具
        Handle(XCAFDoc_ShapeTool) shapeTool = 
            XCAFDoc_DocumentTool::ShapeTool(m_occDoc->Main());
        
        // 遍历所有顶层形状
        TDF_LabelSequence labels;
        shapeTool->GetFreeShapes(labels);
        
        qDebug() << "STEPLoadWorker: 找到" << labels.Length() << "个顶层形状";
        
        int totalShapes = labels.Length();
        int shapeCounter = 0;
        
        for (Standard_Integer i = 1; i <= labels.Length(); i++) {
            TDF_Label label = labels.Value(i);
            TopoDS_Shape shape = shapeTool->GetShape(label);
            
            if (!shape.IsNull()) {
                // 保存顶层形状的名字
                Handle(TDataStd_Name) nameAttr;
                if (label.FindAttribute(TDataStd_Name::GetID(), nameAttr)) {
                    m_topLevelShapeName = QString::fromUtf8(TCollection_AsciiString(nameAttr->Get()).ToCString());
                } else {
                    m_topLevelShapeName = "Model";
                }
                
                processShape(shape, label, shapeCounter);
            }
            
            // 更新进度条
            int progress = 50 + (i * 50 / totalShapes);
            emit progressUpdated(progress, 100, 
                QString("正在处理形状 %1/%2...").arg(i).arg(totalShapes));
        }
        
        qDebug() << "STEPLoadWorker: 模型树构建完成，共" << shapeCounter << "个部件，actorMap大小=" << m_actorMap.size();
        emit progressUpdated(100, 100, "加载完成！");
        emit loadFinished(true, QString("STEP模型加载成功 (%1个部件)").arg(shapeCounter),
                         m_actorMap, m_shapeMap, shapeCounter, m_topLevelShapeName);
        
    } catch (const std::exception& e) {
        qCritical() << "STEPLoadWorker: 加载异常:" << e.what();
        emit loadFinished(false, QString("加载异常: %1").arg(e.what()), m_actorMap, m_shapeMap, 0, "");
    } catch (...) {
        qCritical() << "STEPLoadWorker: 未知异常";
        emit loadFinished(false, "未知异常", m_actorMap, m_shapeMap, 0, "");
    }
}

void STEPLoadWorker::processShape(const TopoDS_Shape& shape, const TDF_Label& label,
                                   int& shapeCounter)
{
    // 获取形状名称
    Handle(TDataStd_Name) nameAttr;
    QString shapeName;
    if (label.FindAttribute(TDataStd_Name::GetID(), nameAttr)) {
        shapeName = QString::fromUtf8(TCollection_AsciiString(nameAttr->Get()).ToCString());
    } else {
        shapeName = QString("Part_%1").arg(++shapeCounter);
    }
    
    // 处理子形状
    Handle(XCAFDoc_ShapeTool) shapeTool = 
        XCAFDoc_DocumentTool::ShapeTool(m_occDoc->Main());
    
    TDF_LabelSequence components;
    bool hasComponents = shapeTool->GetComponents(label, components) && components.Length() > 0;
    
    if (hasComponents) {
        // 有子组件，递归处理子组件
        for (Standard_Integer i = 1; i <= components.Length(); i++) {
            TDF_Label compLabel = components.Value(i);
            TopoDS_Shape compShape = shapeTool->GetShape(compLabel);
            if (!compShape.IsNull()) {
                processShape(compShape, compLabel, shapeCounter);
            }
        }
    } else {
        // 叶子节点，创建VTK Actor
        vtkSmartPointer<vtkActor> actor = createActorFromShape(shape);
        if (actor) {
            m_actorMap[shapeName] = actor;
            m_shapeMap[shapeName] = shape;
            shapeCounter++;  // 递增计数器
            qDebug() << "STEPLoadWorker: 创建Actor成功:" << shapeName << "计数=" << shapeCounter;
        }
    }
}

vtkSmartPointer<vtkActor> STEPLoadWorker::createActorFromShape(const TopoDS_Shape& shape)
{
    // 网格化形状 - 使用更粗糙的网格以加快速度
    BRepMesh_IncrementalMesh mesh(shape, 0.5);
    
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
        
        // 添加顶点
        Standard_Integer nbNodes = triangulation->NbNodes();
        for (Standard_Integer i = 1; i <= nbNodes; i++) {
            gp_Pnt p = triangulation->Node(i).Transformed(transform);
            points->InsertNextPoint(p.X(), p.Y(), p.Z());
        }
        
        // 添加三角形
        Standard_Integer nbTriangles = triangulation->NbTriangles();
        for (Standard_Integer i = 1; i <= nbTriangles; i++) {
            const Poly_Triangle& tri = triangulation->Triangle(i);
            Standard_Integer n1, n2, n3;
            tri.Get(n1, n2, n3);
            
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
