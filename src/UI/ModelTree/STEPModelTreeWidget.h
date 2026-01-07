#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QThread>
#include <QProgressDialog>
#include <memory>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkActor.h>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>

// 前向声明
class STEPLoadWorker;

/**
 * @brief STEP模型树控件（简化版，支持异步加载）
 * 
 * 参考 123/StepViewerWidget 实现，使用异步加载方式避免UI卡顿
 */
class STEPModelTreeWidget : public QWidget {
    Q_OBJECT

public:
    explicit STEPModelTreeWidget(QWidget* parent = nullptr);
    ~STEPModelTreeWidget();

    /**
     * @brief 同步加载STEP文件
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool loadSTEPFile(const QString& filePath);
    
    /**
     * @brief 快速加载STEP文件（使用缓存）
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool loadSTEPFileFast(const QString& filePath);

    /**
     * @brief 清空场景
     */
    void clearScene();

    /**
     * @brief 获取树形视图
     */
    QTreeWidget* getTreeWidget() const { return m_treeWidget; }

    /**
     * @brief 获取OCAF文档
     */
    Handle(TDocStd_Document) getDocument() const { return m_occDoc; }
    
    /**
     * @brief 将所有Actor添加到VTK渲染器
     * @param renderer VTK渲染器
     */
    void addActorsToRenderer(vtkRenderer* renderer);
    
    /**
     * @brief 设置VTK渲染器（用于异步加载完成后添加Actor）
     * @param renderer VTK渲染器
     */
    void setRenderer(vtkRenderer* renderer);
    
    /**
     * @brief 从VTK渲染器移除所有Actor
     * @param renderer VTK渲染器
     */
    void removeActorsFromRenderer(vtkRenderer* renderer);
    
    /**
     * @brief 应用变换到所有Actor
     * @param transform VTK变换
     */
    void applyTransformToAllActors(vtkTransform* transform);
    
    /**
     * @brief 应用变换到指定的Actor
     * @param partName 部件名称
     * @param transform VTK变换
     */
    void applyTransformToActor(const QString& partName, vtkTransform* transform);
    
    /**
     * @brief 设置部件的可见性
     * @param partName 部件名称
     * @param visible 是否可见
     */
    void setPartVisibility(const QString& partName, bool visible);

signals:
    /**
     * @brief 加载完成信号
     * @param success 是否成功
     * @param message 消息
     */
    void loadCompleted(bool success, const QString& message);

    /**
     * @brief 节点可见性改变信号
     * @param partName 部件名称
     * @param visible 是否可见
     */
    void partVisibilityChanged(const QString& partName, bool visible);
    
    /**
     * @brief 进度更新信号
     * @param current 当前进度
     * @param total 总进度
     * @param message 消息
     */
    void progressUpdated(int current, int total, const QString& message);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);
    void onLoadProgress(int current, int total, const QString& message);
    void onLoadFinished(bool success, const QString& message,
                        QMap<QString, vtkSmartPointer<vtkActor>> actors,
                        QMap<QString, TopoDS_Shape> shapes,
                        int shapeCounter, const QString& topLevelName);

private:
    void setupUI();
    void setupContextMenu();
    void processShape(const TopoDS_Shape& shape, const TDF_Label& label, 
                      QTreeWidgetItem* parentItem = nullptr);
    vtkSmartPointer<vtkActor> createActorFromShape(const TopoDS_Shape& shape);
    
    // 辅助函数：递归设置可见性
    void setItemVisibilityRecursive(QTreeWidgetItem* item, bool visible);
    
    // 辅助函数：递归高亮
    void highlightItemRecursive(QTreeWidgetItem* item);
    
    // 缓存相关
    QString getCachePath(const QString& stepFilePath);
    bool isCacheValid(const QString& cachePath, const QString& stepFilePath);
    bool saveToCache(const QString& cachePath);
    bool loadFromCache(const QString& cachePath);
    
    // 树结构保存/加载
    bool saveTreeStructure(const QString& jsonPath);
    bool loadTreeStructure(const QString& jsonPath);
    QJsonObject treeItemToJson(QTreeWidgetItem* item);
    QTreeWidgetItem* jsonToTreeItem(const QJsonObject& json, QTreeWidgetItem* parent = nullptr);

private:
    QVBoxLayout* m_layout;
    QTreeWidget* m_treeWidget;
    QLabel* m_statusLabel;
    
    // OpenCASCADE文档
    Handle(TDocStd_Document) m_occDoc;
    
    // Actor映射
    QMap<QString, vtkSmartPointer<vtkActor>> m_actorMap;
    QMap<QString, TopoDS_Shape> m_shapeMap;
    
    // 形状计数器
    int m_shapeCounter;
    
    // 上下文菜单
    QMenu* m_contextMenu;
    QAction* m_expandAction;
    QAction* m_collapseAction;
    
    // 异步加载相关
    QThread* m_loadThread;
    STEPLoadWorker* m_loadWorker;
    QProgressDialog* m_progressDialog;
    QString m_currentCachePath;  // 当前加载的缓存路径
    vtkRenderer* m_renderer;  // VTK渲染器引用
};