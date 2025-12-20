#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// OpenCASCADE基础测试
#include <TopoDS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPCAFControl_Reader.hxx>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "OpenCASCADE基础功能测试" << std::endl;
    std::cout << "======================" << std::endl;

    try {
        // 测试1: 创建一个简单的立方体
        std::cout << "测试1: 创建立方体..." << std::endl;
        BRepPrimAPI_MakeBox boxMaker(10.0, 10.0, 10.0);
        TopoDS_Shape box = boxMaker.Shape();
        
        if (!box.IsNull()) {
            std::cout << "✓ 立方体创建成功" << std::endl;
        } else {
            std::cout << "✗ 立方体创建失败" << std::endl;
            return -1;
        }

        // 测试2: 创建STEP读取器
        std::cout << "测试2: 创建STEP读取器..." << std::endl;
        STEPCAFControl_Reader reader;
        std::cout << "✓ STEP读取器创建成功" << std::endl;

        // 测试3: 检查文件是否存在
        QString testFile = "K:/vsCodeProjects/qtSpraySystem/data/model/MPX3500.STEP";
        std::cout << "测试3: 检查文件 " << testFile.toStdString() << std::endl;
        
        QFileInfo fileInfo(testFile);
        if (fileInfo.exists()) {
            std::cout << "✓ 文件存在，大小: " << fileInfo.size() << " 字节" << std::endl;
            
            // 测试4: 尝试读取文件头部
            std::cout << "测试4: 尝试读取STEP文件..." << std::endl;
            IFSelect_ReturnStatus status = reader.ReadFile(testFile.toLocal8Bit().constData());
            
            switch (status) {
                case IFSelect_RetDone:
                    std::cout << "✓ STEP文件读取成功" << std::endl;
                    break;
                case IFSelect_RetError:
                    std::cout << "✗ STEP文件读取错误" << std::endl;
                    break;
                case IFSelect_RetFail:
                    std::cout << "✗ STEP文件读取失败" << std::endl;
                    break;
                case IFSelect_RetVoid:
                    std::cout << "✗ STEP文件为空" << std::endl;
                    break;
                default:
                    std::cout << "✗ 未知错误" << std::endl;
                    break;
            }
        } else {
            std::cout << "✗ 文件不存在" << std::endl;
        }

        std::cout << "=== 测试完成 ===" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return -1;
    }
}