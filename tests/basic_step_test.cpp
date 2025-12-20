#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// 最基础的STEP测试，不使用XCAF
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <IFSelect_ReturnStatus.hxx>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== 基础STEP读取测试 ===";
    
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
        qDebug() << "1. 创建基础STEP读取器...";
        STEPControl_Reader reader;
        
        qDebug() << "2. 读取STEP文件...";
        std::string pathStr = testFile.toStdString();
        
        qDebug() << "   文件路径:" << QString::fromStdString(pathStr);
        
        IFSelect_ReturnStatus status = reader.ReadFile(pathStr.c_str());
        
        qDebug() << "   读取状态:" << (int)status;
        
        if (status != IFSelect_RetDone) {
            qCritical() << "STEP文件读取失败，状态:" << (int)status;
            switch (status) {
                case IFSelect_RetError:
                    qCritical() << "   错误: IFSelect_RetError";
                    break;
                case IFSelect_RetFail:
                    qCritical() << "   错误: IFSelect_RetFail";
                    break;
                case IFSelect_RetVoid:
                    qCritical() << "   错误: IFSelect_RetVoid (文件为空)";
                    break;
                default:
                    qCritical() << "   错误: 未知状态";
                    break;
            }
            return -1;
        }
        
        qDebug() << "3. 传输根对象...";
        reader.TransferRoots();
        
        qDebug() << "4. 获取形状...";
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            qCritical() << "没有找到有效的几何体";
            return -1;
        }
        
        qDebug() << "成功！找到有效的几何体";
        qDebug() << "形状类型:" << shape.ShapeType();
        
        qDebug() << "=== 基础测试成功完成 ===";
        return 0;
        
    } catch (const std::exception& e) {
        qCritical() << "异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "未知异常";
        return -1;
    }
}