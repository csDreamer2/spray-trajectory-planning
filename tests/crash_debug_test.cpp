#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// 包含我们的STEP模型树类
#include "../src/Data/STEPModelTree.h"

/**
 * @brief 崩溃调试测试程序
 */
class CrashDebugTest : public QObject {
    Q_OBJECT

public:
    CrashDebugTest(QObject* parent = nullptr) : QObject(parent) {
        m_modelTree = new STEPModelTree(this);
        
        // 连接信号
        connect(m_modelTree, &STEPModelTree::modelTreeLoaded,
                this, &CrashDebugTest::onModelTreeLoaded);
        connect(m_modelTree, &STEPModelTree::loadProgress,
                this, &CrashDebugTest::onLoadProgress);
    }

    bool testSTEPFile(const QString& filePath) {
        std::cout << "=== 崩溃调试测试 ===" << std::endl;
        std::cout << "测试文件: " << filePath.toStdString() << std::endl;
        
        if (!QFileInfo::exists(filePath)) {
            std::cout << "错误: 文件不存在!" << std::endl;
            return false;
        }

        std::cout << "开始加载..." << std::endl;
        m_loadSuccess = false;
        
        try {
            bool result = m_modelTree->loadFromSTEPFile(filePath);
            std::cout << "loadFromSTEPFile 返回: " << (result ? "true" : "false") << std::endl;
            
            // 等待信号
            QCoreApplication::processEvents();
            
            return result;
        } catch (const std::exception& e) {
            std::cout << "捕获异常: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cout << "捕获未知异常" << std::endl;
            return false;
        }
    }

private slots:
    void onModelTreeLoaded(bool success, const QString& message) {
        m_loadSuccess = success;
        
        if (success) {
            std::cout << "✓ 加载成功: " << message.toStdString() << std::endl;
            
            // 测试基本功能
            testBasicFunctions();
        } else {
            std::cout << "✗ 加载失败: " << message.toStdString() << std::endl;
        }
        
        QCoreApplication::quit();
    }

    void onLoadProgress(int progress, const QString& message) {
        std::cout << "进度 " << progress << "%: " << message.toStdString() << std::endl;
    }

private:
    void testBasicFunctions() {
        std::cout << "\n=== 基本功能测试 ===" << std::endl;
        
        try {
            // 测试统计信息
            auto stats = m_modelTree->getModelStats();
            std::cout << "模型统计:" << std::endl;
            std::cout << "  总节点: " << stats.totalNodes << std::endl;
            std::cout << "  装配体: " << stats.assemblies << std::endl;
            std::cout << "  零件: " << stats.parts << std::endl;
            std::cout << "  最大深度: " << stats.maxDepth << std::endl;
            std::cout << "  可见节点: " << stats.visibleNodes << std::endl;

            // 测试根节点
            auto rootNode = m_modelTree->getRootNode();
            if (rootNode) {
                std::cout << "根节点: " << rootNode->name.toStdString() << std::endl;
                std::cout << "子节点数: " << rootNode->children.size() << std::endl;
            }

            std::cout << "=== 测试完成 ===" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "基本功能测试异常: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "基本功能测试未知异常" << std::endl;
        }
    }

private:
    STEPModelTree* m_modelTree;
    bool m_loadSuccess;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "STEP模型树崩溃调试测试程序" << std::endl;
    std::cout << "============================" << std::endl;

    // 检查命令行参数
    QString testFile;
    if (argc > 1) {
        testFile = QString::fromLocal8Bit(argv[1]);
    } else {
        // 使用默认测试文件
        testFile = "data/model/MPX3500.STEP";
        if (!QFileInfo::exists(testFile)) {
            testFile = "../data/model/MPX3500.STEP";
        }
    }

    try {
        CrashDebugTest test;
        
        if (test.testSTEPFile(testFile)) {
            std::cout << "\n程序将在加载完成后自动退出..." << std::endl;
            return app.exec();
        } else {
            std::cout << "测试失败" << std::endl;
            return -1;
        }
    }
    catch (const std::exception& e) {
        std::cout << "主程序异常: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cout << "主程序未知异常" << std::endl;
        return -1;
    }
}

#include "crash_debug_test.moc"