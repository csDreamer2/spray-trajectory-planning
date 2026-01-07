#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QMap>
#include <QString>
#include <memory>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkActor.h>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>

/**
 * @brief STEP模型树控件（简化版，同步加载）
 * 
 * 参考 123/StepViewerWidget 实现，使用同步加载方式
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

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);

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
};