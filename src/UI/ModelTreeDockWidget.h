#pragma once

#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>

#include "STEPModelTreeWidget.h"

/**
 * @brief 模型树停靠窗口
 * 
 * 集成STEP模型树功能的停靠窗口，提供完整的模型管理界面
 */
class ModelTreeDockWidget : public QDockWidget {
    Q_OBJECT

public:
    explicit ModelTreeDockWidget(QWidget* parent = nullptr);
    ~ModelTreeDockWidget();

    /**
     * @brief 加载STEP文件到模型树
     * @param filePath 文件路径
     */
    void loadSTEPFile(const QString& filePath);

    /**
     * @brief 获取模型树控件
     * @return STEPModelTreeWidget指针
     */
    STEPModelTreeWidget* getModelTreeWidget() const { return m_modelTreeWidget; }

signals:
    /**
     * @brief 请求重新渲染信号
     */
    void renderUpdateRequested();

private slots:
    void onModelTreeLoaded(bool success, const QString& message);
    void onPartVisibilityChanged(const QString& partName, bool visible);
    
    // 控制按钮槽函数
    void onShowAllClicked();
    void onHideAllClicked();
    void onExpandAllClicked();
    void onCollapseAllClicked();
    void onRefreshClicked();

private:
    void setupUI();
    void setupControlPanel();
    void setupDisplayOptions();
    void updateStatistics();

private:
    // 主要组件
    STEPModelTreeWidget* m_modelTreeWidget;
    QWidget* m_mainWidget;
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;

    // 控制面板
    QGroupBox* m_controlGroup;
    QHBoxLayout* m_controlLayout;
    QPushButton* m_showAllButton;
    QPushButton* m_hideAllButton;
    QPushButton* m_expandAllButton;
    QPushButton* m_collapseAllButton;
    QPushButton* m_refreshButton;

    // 显示选项
    QGroupBox* m_displayGroup;
    QVBoxLayout* m_displayLayout;
    QCheckBox* m_showAssembliesCheck;
    QCheckBox* m_showPartsCheck;
    QLabel* m_transparencyLabel;
    QSlider* m_transparencySlider;
    QSpinBox* m_transparencySpin;

    // 状态信息
    QGroupBox* m_statusGroup;
    QVBoxLayout* m_statusLayout;
    QLabel* m_totalNodesLabel;
    QLabel* m_visibleNodesLabel;
    QLabel* m_selectedNodesLabel;
    QProgressBar* m_loadProgressBar;
    QLabel* m_loadStatusLabel;

    // 当前状态
    QString m_currentFilePath;
    bool m_isLoading;
    int m_totalNodes;
    int m_visibleNodes;
    int m_selectedNodes;
};