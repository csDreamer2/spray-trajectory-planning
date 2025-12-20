#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>

// 包含我们的STEP模型树类
#include "../src/Data/STEPModelTree.h"

/**
 * @brief 简单的STEP模型树测试程序（无GUI）
 */
class SimpleSTEPTest : public QObject {
    Q_OBJECT

public:
    SimpleSTEPTest(QObject* parent = nullptr) : QObject(parent) {
        m_modelTree = new STEPModelTree(this);
        
        // 连接信号
        connect(m_modelTree, &STEPModelTree::modelTreeLoaded,
                this, &SimpleSTEPTest::onModelTreeLoaded);
        connect(m_modelTree, &STEPModelTree::loadProgress,
                this, &SimpleSTEPTest::onLoadProgress);
    }

    bool testSTEPFile(const QString& filePath) {
        std::cout << "=== STEP模型树功能测试 ===" << std::endl;
        std::cout << "测试文件: " << filePath.toStdString() << std::endl;
        
        if (!QFileInfo::exists(filePath)) {
            std::cout << "错误: 文件不存在!" << std::endl;
            return false;
        }

        std::cout << "开始加载..." << std::endl;
        m_loadSuccess = false;
        m_modelTree->loadFromSTEPFile(filePath);
        
        // 简单等待加载完成（在实际应用中应该使用事件循环）
        QCoreApplication::processEvents();
        
        return m_loadSuccess;
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
            
            // 显示前几个子节点
            int count = 0;
            for (const auto& child : rootNode->children) {
                if (count >= 5) break;
                std::cout << "  - " << child->name.toStdString() 
                         << " (" << (child->isAssembly ? "装配体" : "零件") << ")" << std::endl;
                count++;
            }
            
            if (rootNode->children.size() > 5) {
                std::cout << "  ... 还有 " << (rootNode->children.size() - 5) << " 个子节点" << std::endl;
            }
        }

        // 测试查找功能
        auto nodes = m_modelTree->findNodesByName("MPX3500");
        std::cout << "查找 'MPX3500': 找到 " << nodes.size() << " 个节点" << std::endl;

        // 测试可见形状
        auto visibleShapes = m_modelTree->getVisibleShapes();
        std::cout << "可见形状数量: " << visibleShapes.size() << std::endl;

        std::cout << "=== 测试完成 ===" << std::endl;
    }

private:
    STEPModelTree* m_modelTree;
    bool m_loadSuccess;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "STEP模型树简单测试程序" << std::endl;
    std::cout << "========================" << std::endl;

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
        SimpleSTEPTest test;
        
        if (test.testSTEPFile(testFile)) {
            std::cout << "\n程序将在加载完成后自动退出..." << std::endl;
            return app.exec();
        } else {
            std::cout << "测试失败" << std::endl;
            return -1;
        }
    }
    catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return -1;
    }
}

#include "simple_step_test.moc"