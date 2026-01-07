#pragma once

#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QVariant>
#include <memory>
#include <vector>
#include <map>
#include <string>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_Name.hxx>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkActor.h>

/**
 * @brief STEP文件模型树节点
 * 
 * 表示STEP文件中的一个组件或装配体
 */
struct STEPTreeNode {
    QString name;                    // 组件名称
    QString label;                   // STEP标签
    TopoDS_Shape shape;             // 几何形状
    TDF_Label stepLabel;            // STEP文档标签
    bool isVisible;                 // 是否可见
    bool isAssembly;                // 是否为装配体
    int level;                      // 层级深度
    std::vector<std::shared_ptr<STEPTreeNode>> children; // 子节点
    std::weak_ptr<STEPTreeNode> parent;                  // 父节点
    vtkSmartPointer<vtkActor> actor; // VTK Actor用于3D显示
    
    STEPTreeNode() : isVisible(true), isAssembly(false), level(0), actor(nullptr) {}
};

/**
 * @brief STEP模型树管理器
 * 
 * 负责解析STEP文件的层次结构，构建模型树，并管理组件的显示状态
 */
class STEPModelTree : public QObject {
    Q_OBJECT

public:
    explicit STEPModelTree(QObject* parent = nullptr);
    ~STEPModelTree();

    /**
     * @brief 从STEP文件加载模型树
     * @param filePath STEP文件路径
     * @return 是否加载成功
     */
    bool loadFromSTEPFile(const QString& filePath);

    /**
     * @brief 获取模型树的根节点
     * @return 根节点指针
     */
    std::shared_ptr<STEPTreeNode> getRootNode() const { return m_rootNode; }

    /**
     * @brief 获取Qt模型用于树形控件显示
     * @return QStandardItemModel指针
     */
    QStandardItemModel* getQtModel() const { return m_qtModel; }

    /**
     * @brief 设置节点的可见性
     * @param node 节点指针
     * @param visible 是否可见
     * @param recursive 是否递归设置子节点
     */
    void setNodeVisibility(std::shared_ptr<STEPTreeNode> node, bool visible, bool recursive = false);

    /**
     * @brief 根据名称查找节点
     * @param name 节点名称
     * @return 找到的节点列表
     */
    std::vector<std::shared_ptr<STEPTreeNode>> findNodesByName(const QString& name) const;

    /**
     * @brief 获取所有可见的形状
     * @return 可见形状列表
     */
    std::vector<TopoDS_Shape> getVisibleShapes() const;

    /**
     * @brief 获取指定节点的完整路径
     * @param node 节点指针
     * @return 路径字符串
     */
    QString getNodePath(std::shared_ptr<STEPTreeNode> node) const;

    /**
     * @brief 展开/折叠所有节点
     * @param expand true为展开，false为折叠
     */
    void expandAll(bool expand = true);

    /**
     * @brief 获取模型统计信息
     */
    struct ModelStats {
        int totalNodes;
        int visibleNodes;
        int assemblies;
        int parts;
        int maxDepth;
    };
    ModelStats getModelStats() const;

signals:
    /**
     * @brief 节点可见性改变信号
     * @param node 改变的节点
     * @param visible 新的可见状态
     */
    void nodeVisibilityChanged(std::shared_ptr<STEPTreeNode> node, bool visible);

    /**
     * @brief 模型树加载完成信号
     * @param success 是否成功
     * @param message 消息
     */
    void modelTreeLoaded(bool success, const QString& message);

    /**
     * @brief 加载进度信号
     * @param progress 进度百分比 (0-100)
     * @param message 当前操作描述
     */
    void loadProgress(int progress, const QString& message);

private slots:
    void onItemChanged(QStandardItem* item);

private:
    /**
     * @brief 递归清理节点的OpenCASCADE引用
     */
    void clearNodeReferences(std::shared_ptr<STEPTreeNode> node);

    /**
     * @brief 递归解析STEP文档标签
     * @param label STEP标签
     * @param parent 父节点
     * @param level 当前层级
     * @param maxDepth 最大递归深度，防止栈溢出
     */
    void parseSTEPLabel(const TDF_Label& label, 
                       std::shared_ptr<STEPTreeNode> parent, 
                       int level = 0,
                       int maxDepth = 100);

    /**
     * @brief 解析复合形状，提取子形状
     * @param compoundShape 复合形状
     * @param parent 父节点
     * @param level 当前层级
     * @param maxDepth 最大递归深度
     */
    void parseCompoundShape(const TopoDS_Shape& compoundShape,
                           std::shared_ptr<STEPTreeNode> parent,
                           int level,
                           int maxDepth);

    /**
     * @brief 在STEP文档中查找形状对应的名称
     * @param shape 要查找的形状
     * @return 找到的名称，如果没找到返回空字符串
     */
    QString findShapeNameInDocument(const TopoDS_Shape& shape) const;

    /**
     * @brief 从STEP标签创建树节点
     * @param label STEP标签
     * @param level 层级
     * @return 创建的节点
     */
    std::shared_ptr<STEPTreeNode> createNodeFromLabel(const TDF_Label& label, int level);

    /**
     * @brief 构建Qt模型项
     * @param node 树节点
     * @param parentItem 父Qt项
     */
    void buildQtModelItem(std::shared_ptr<STEPTreeNode> node, QStandardItem* parentItem);

    /**
     * @brief 递归收集可见形状
     * @param node 当前节点
     * @param shapes 形状列表
     */
    void collectVisibleShapes(std::shared_ptr<STEPTreeNode> node, 
                             std::vector<TopoDS_Shape>& shapes) const;

    /**
     * @brief 递归计算统计信息
     * @param node 当前节点
     * @param stats 统计信息
     */
    void calculateStats(std::shared_ptr<STEPTreeNode> node, ModelStats& stats) const;

    /**
     * @brief 根据指针查找节点
     * @param nodePtr 节点指针
     * @return 找到的shared_ptr节点
     */
    std::shared_ptr<STEPTreeNode> findNodeByPointer(STEPTreeNode* nodePtr) const;

    /**
     * @brief 在树中递归查找节点
     * @param current 当前节点
     * @param target 目标指针
     * @return 找到的节点
     */
    std::shared_ptr<STEPTreeNode> findNodeInTreeByPointer(
        std::shared_ptr<STEPTreeNode> current, STEPTreeNode* target) const;

    /**
     * @brief 获取STEP标签的名称
     * @param label STEP标签
     * @return 名称字符串
     */
    QString getLabelName(const TDF_Label& label) const;

    /**
     * @brief 检查标签是否为装配体
     * @param label STEP标签
     * @return 是否为装配体
     */
    bool isAssemblyLabel(const TDF_Label& label) const;

private:
    Handle(TDocStd_Document) m_stepDocument;        // STEP文档
    Handle(XCAFDoc_ShapeTool) m_shapeTool;          // 形状工具
    Handle(XCAFDoc_ColorTool) m_colorTool;          // 颜色工具
    Handle(XCAFDoc_LayerTool) m_layerTool;          // 图层工具
    
    std::shared_ptr<STEPTreeNode> m_rootNode;       // 根节点
    QStandardItemModel* m_qtModel;                  // Qt模型
    
    // 节点映射，用于快速查找 - 使用字符串作为键而不是TDF_Label
    std::map<std::string, std::shared_ptr<STEPTreeNode>> m_labelToNode;
    std::map<QString, std::vector<std::shared_ptr<STEPTreeNode>>> m_nameToNodes;
    
    bool m_isLoading;                               // 是否正在加载
    int m_totalLabels;                              // 总标签数
    int m_processedLabels;                          // 已处理标签数
};