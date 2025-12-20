#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <iostream>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QString testFile = "../../../test_data/test_points.ply";
    
    qDebug() << "=== VTK PLY 读取测试 ===";
    qDebug() << "测试文件:" << testFile;
    
    // 检查文件存在
    QFileInfo fileInfo(testFile);
    qDebug() << "文件存在:" << fileInfo.exists();
    qDebug() << "绝对路径:" << fileInfo.absoluteFilePath();
    qDebug() << "文件大小:" << fileInfo.size() << "字节";
    qDebug() << "是否可读:" << fileInfo.isReadable();
    
    if (!fileInfo.exists()) {
        qCritical() << "❌ 测试文件不存在";
        return -1;
    }
    
    // 测试VTK读取
    try {
        vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
        
        // 方法1：使用原始路径
        qDebug() << "\n--- 方法1：原始路径 ---";
        reader->SetFileName(testFile.toLocal8Bit().data());
        reader->Update();
        
        if (reader->GetOutput() && reader->GetOutput()->GetNumberOfPoints() > 0) {
            qDebug() << "✅ 方法1成功，点数:" << reader->GetOutput()->GetNumberOfPoints();
        } else {
            qDebug() << "❌ 方法1失败";
            
            // 方法2：使用绝对路径
            qDebug() << "\n--- 方法2：绝对路径 ---";
            QString absolutePath = fileInfo.absoluteFilePath();
            reader->SetFileName(absolutePath.toLocal8Bit().data());
            reader->Update();
            
            if (reader->GetOutput() && reader->GetOutput()->GetNumberOfPoints() > 0) {
                qDebug() << "✅ 方法2成功，点数:" << reader->GetOutput()->GetNumberOfPoints();
            } else {
                qDebug() << "❌ 方法2失败";
                
                // 方法3：使用标准路径格式
                qDebug() << "\n--- 方法3：标准路径格式 ---";
                QString standardPath = absolutePath.replace("\\", "/");
                qDebug() << "标准路径:" << standardPath;
                reader->SetFileName(standardPath.toStdString().c_str());
                reader->Update();
                
                if (reader->GetOutput() && reader->GetOutput()->GetNumberOfPoints() > 0) {
                    qDebug() << "✅ 方法3成功，点数:" << reader->GetOutput()->GetNumberOfPoints();
                } else {
                    qDebug() << "❌ 方法3失败";
                    qDebug() << "VTK错误信息已输出到控制台";
                }
            }
        }
        
    } catch (const std::exception& e) {
        qCritical() << "异常:" << e.what();
    }
    
    qDebug() << "\n=== 测试完成 ===";
    return 0;
}