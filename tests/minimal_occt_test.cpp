#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// 最小OpenCASCADE STEP测试
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_LabelSequence.hxx>
#include <IFSelect_ReturnStatus.hxx>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== 最小OpenCASCADE STEP测试 ===";
    
    // 测试文件路径
    QString testFile = "data/model/MPX3500.STEP";
    QFileInfo fileInfo(testFile);
    
    if (!fileInfo.exists()) {
        qCritical() << "测试文件不存在:" << testFile;
        return -1;
    }
    
    qDebug() << "测试文件:" << testFile;
    qDebug() << "文件大小:" << fileInfo.size() << "bytes";
    
    try {
        qDebug() << "1. 创建STEP读取器...";
        STEPCAFControl_Reader reader;
        
        qDebug() << "2. 创建XCAF应用程序...";
        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        if (app.IsNull()) {
            qCritical() << "无法获取XCAFApp_Application";
            return -1;
        }
        
        qDebug() << "3. 创建XCAF文档...";
        Handle(TDocStd_Document) doc;
        app->NewDocument("MDTV-XCAF", doc);
        if (doc.IsNull()) {
            qCritical() << "无法创建XCAF文档";
            return -1;
        }
        
        qDebug() << "4. 读取STEP文件...";
        std::string pathStr = testFile.toStdString();
        IFSelect_ReturnStatus status = reader.ReadFile(pathStr.c_str());
        
        if (status != IFSelect_RetDone) {
            qCritical() << "STEP文件读取失败，状态:" << (int)status;
            return -1;
        }
        
        qDebug() << "5. 传输数据到文档...";
        if (!reader.Transfer(doc)) {
            qCritical() << "数据传输失败";
            return -1;
        }
        
        qDebug() << "6. 获取形状工具...";
        Handle(XCAFDoc_ShapeTool) shapeTool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
        if (shapeTool.IsNull()) {
            qCritical() << "无法获取形状工具";
            return -1;
        }
        
        qDebug() << "7. 获取自由形状...";
        TDF_LabelSequence freeShapes;
        shapeTool->GetFreeShapes(freeShapes);
        
        qDebug() << "成功！找到" << freeShapes.Length() << "个自由形状";
        
        // 简单遍历，不做复杂解析
        for (int i = 1; i <= freeShapes.Length() && i <= 5; i++) {
            TDF_Label label = freeShapes.Value(i);
            qDebug() << "  形状" << i << ": 是装配体?" << shapeTool->IsAssembly(label);
        }
        
        qDebug() << "8. 清理资源...";
        shapeTool.Nullify();
        app->Close(doc);
        doc.Nullify();
        
        qDebug() << "=== 测试成功完成 ===";
        return 0;
        
    } catch (const std::exception& e) {
        qCritical() << "异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "未知异常";
        return -1;
    }
}