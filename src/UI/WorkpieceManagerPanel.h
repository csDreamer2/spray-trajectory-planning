#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <QAction>

namespace UI {

/**
 * @brief 工件管理面板
 * 
 * 显示data/pointclouds目录下的点云文件列表
 * 支持添加、删除工件，双击可视化
 */
class WorkpieceManagerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit WorkpieceManagerPanel(QWidget* parent = nullptr);
    ~WorkpieceManagerPanel();

    /**
     * @brief 刷新工件列表
     */
    void refreshWorkpieceList();

    /**
     * @brief 获取选中的工件路径
     */
    QString getSelectedWorkpiecePath() const;

signals:
    /**
     * @brief 工件被双击，请求可视化
     * @param filePath 工件文件路径
     */
    void workpieceDoubleClicked(const QString& filePath);

    /**
     * @brief 工件被选中
     * @param filePath 工件文件路径
     */
    void workpieceSelected(const QString& filePath);

    /**
     * @brief 工件列表已更新
     * @param count 工件数量
     */
    void workpieceListUpdated(int count);

private slots:
    void onAddWorkpiece();
    void onDeleteWorkpiece();
    void onRefreshList();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onItemSelectionChanged();
    void onContextMenuRequested(const QPoint& pos);
    void onShowInExplorer();
    void onRenameWorkpiece();

private:
    void setupUI();
    void setupConnections();
    void loadWorkpieceList();
    QString getWorkpieceDirectory() const;
    QString formatFileSize(qint64 bytes) const;
    QIcon getFileIcon(const QString& extension) const;

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    
    QLabel* m_titleLabel;
    QLabel* m_countLabel;
    QListWidget* m_workpieceList;
    
    QPushButton* m_addButton;
    QPushButton* m_deleteButton;
    QPushButton* m_refreshButton;
    
    QMenu* m_contextMenu;
    QAction* m_visualizeAction;
    QAction* m_deleteAction;
    QAction* m_renameAction;
    QAction* m_showInExplorerAction;
    
    QString m_workpieceDirectory;
};

} // namespace UI
