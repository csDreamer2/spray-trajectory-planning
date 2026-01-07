#ifndef PARAMETERPANEL_H
#define PARAMETERPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>

namespace UI {

/**
 * @brief 参数配置面板
 * 
 * 提供喷涂参数的配置界面
 */
class ParameterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget *parent = nullptr);

signals:
    void parametersChanged();

private slots:
    void onParameterChanged();
    void onLoadTemplate();
    void onSaveTemplate();

private:
    void setupUI();

private:
    QVBoxLayout* m_mainLayout;
    QGroupBox* m_sprayGroup;
    QGroupBox* m_trajectoryGroup;
    QGroupBox* m_qualityGroup;
    
    // 喷涂参数控件
    QDoubleSpinBox* m_pressureSpinBox;
    QDoubleSpinBox* m_flowRateSpinBox;
    QDoubleSpinBox* m_sprayHeightSpinBox;
    QDoubleSpinBox* m_spraySpeedSpinBox;
    QComboBox* m_materialComboBox;
    
    // 轨迹参数控件
    QDoubleSpinBox* m_overlapRateSpinBox;
    QSpinBox* m_passCountSpinBox;
    
    // 质量参数控件
    QDoubleSpinBox* m_targetThicknessSpinBox;
    QDoubleSpinBox* m_coverageRateSpinBox;
    
    // 按钮
    QPushButton* m_loadTemplateButton;
    QPushButton* m_saveTemplateButton;
};

} // namespace UI

#endif // PARAMETERPANEL_H