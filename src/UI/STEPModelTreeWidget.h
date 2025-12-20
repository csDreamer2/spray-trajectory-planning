#pragma once

#include <QWidget>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QProgressBar>
#include <QLabel>
#include <QThread>
#include <memory>
#include <vector>

#include "../Data/STEPModelTree.h"

class STEPModelTreeWorker;

/**
 * @brief STEP模型树控件
 * 
 * 集成了树形视图和模型管理的完整控件，支持异步加载
 */
class STEPModelTreeWidget : public QWidget {
    Q_OBJECT

public:
    explicit STEPModelTreeWidget(QWidget* parent = nullptr);
    ~STEPModelTreeWidget();

    /**
     * @brief 异步加载STEP文件
     * @param filePath 文件路径
     */
    void loadSTEPFile(const QString& filePath);

    /**
     * @brief 取消当前加载
     */
    void cancelLoading();

    /**
     * @brief 获取模型树管理器
     * @return STEPModelTree指针
     */
    STEPModelTree* getModelTree() const { return m_modelTree; }

    /**
     * @brief 获取树形视图
     * @return QTreeView指针
     */
    QTreeView* getTreeView() const { return m_treeView; }

    /**
     * @brief 检查是否正在加载
     * @return 是否正在加载
     */
    bool isLoading() const { return m_isLoading; }

signals:
    /**
     * @brief 选中节点改变信号
     * @param selectedNodes 选中的节点列表
     */
    void selectionChanged(const std::vector<std::shared_ptr<STEPTreeNode>>& selectedNodes);

    /**
     * @brief 可见性改变信号
     * @param visibleShapes 当前可见的形状列表
     */
    void visibilityChanged(const std::vector<TopoDS_Shape>& visibleShapes);

    /**
     * @brief 加载完成信号
     * @param success 是否成功
     * @param message 消息
     */
    void loadCompleted(bool success, const QString& message);

private slots:
    void onSelectionChanged();
    void onNodeVisibilityChanged(std::shared_ptr<STEPTreeNode> node, bool visible);
    void onModelTreeLoaded(bool success, const QString& message);
    void onContextMenuRequested(const QPoint& pos);
    
    // 异步加载相关槽函数
    void onWorkerProgressUpdate(int progress, const QString& message);
    void onWorkerModelTreeLoaded(bool success, const QString& message, std::shared_ptr<STEPTreeNode> rootNode);
    void onWorkerLoadFailed(const QString& error);

private:
    void setupUI();
    void setupContextMenu();
    void setupWorker();
    void showLoadingUI(bool show);
    void updateModelFromWorkerResult(std::shared_ptr<STEPTreeNode> rootNode);
    void buildQtModelItemFromNode(std::shared_ptr<STEPTreeNode> node, QStandardItem* parentItem);
    
    std::shared_ptr<STEPTreeNode> getNodeFromItem(QStandardItem* item) const;
    std::shared_ptr<STEPTreeNode> findNodeInTree(std::shared_ptr<STEPTreeNode> current, STEPTreeNode* target) const;

private:
    STEPModelTree* m_modelTree;
    QTreeView* m_treeView;
    QVBoxLayout* m_layout;
    
    // 加载进度UI
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    // 异步加载
    QThread* m_workerThread;
    STEPModelTreeWorker* m_worker;
    bool m_isLoading;
    
    // 上下文菜单
    QMenu* m_contextMenu;
    QAction* m_showAction;
    QAction* m_hideAction;
    QAction* m_showOnlyAction;
    QAction* m_expandAction;
    QAction* m_collapseAction;
    QAction* m_propertiesAction;
};