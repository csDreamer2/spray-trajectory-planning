#include "ParameterPanel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>

namespace UI {

ParameterPanel::ParameterPanel(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
{
    setupUI();
}

void ParameterPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 喷涂参数组
    m_sprayGroup = new QGroupBox("喷涂参数", this);
    QFormLayout* sprayLayout = new QFormLayout(m_sprayGroup);
    
    m_pressureSpinBox = new QDoubleSpinBox();
    m_pressureSpinBox->setRange(0.1, 10.0);
    m_pressureSpinBox->setValue(2.5);
    m_pressureSpinBox->setSuffix(" bar");
    sprayLayout->addRow("喷涂压力:", m_pressureSpinBox);
    
    m_flowRateSpinBox = new QDoubleSpinBox();
    m_flowRateSpinBox->setRange(10, 1000);
    m_flowRateSpinBox->setValue(200);
    m_flowRateSpinBox->setSuffix(" ml/min");
    sprayLayout->addRow("流量:", m_flowRateSpinBox);
    
    m_sprayHeightSpinBox = new QDoubleSpinBox();
    m_sprayHeightSpinBox->setRange(50, 500);
    m_sprayHeightSpinBox->setValue(150);
    m_sprayHeightSpinBox->setSuffix(" mm");
    sprayLayout->addRow("喷涂高度:", m_sprayHeightSpinBox);
    
    m_materialComboBox = new QComboBox();
    m_materialComboBox->addItems({"环氧富锌漆", "有机硅耐高温漆"});
    sprayLayout->addRow("涂料类型:", m_materialComboBox);
    
    // 轨迹参数组
    m_trajectoryGroup = new QGroupBox("轨迹参数", this);
    QFormLayout* trajectoryLayout = new QFormLayout(m_trajectoryGroup);
    
    m_overlapRateSpinBox = new QDoubleSpinBox();
    m_overlapRateSpinBox->setRange(10, 80);
    m_overlapRateSpinBox->setValue(30);
    m_overlapRateSpinBox->setSuffix(" %");
    trajectoryLayout->addRow("重叠率:", m_overlapRateSpinBox);
    
    // 质量参数组
    m_qualityGroup = new QGroupBox("质量参数", this);
    QFormLayout* qualityLayout = new QFormLayout(m_qualityGroup);
    
    m_targetThicknessSpinBox = new QDoubleSpinBox();
    m_targetThicknessSpinBox->setRange(10, 200);
    m_targetThicknessSpinBox->setValue(75);
    m_targetThicknessSpinBox->setSuffix(" μm");
    qualityLayout->addRow("目标膜厚:", m_targetThicknessSpinBox);
    
    // 按钮
    m_loadTemplateButton = new QPushButton("加载模板");
    m_saveTemplateButton = new QPushButton("保存模板");
    
    // 布局
    m_mainLayout->addWidget(m_sprayGroup);
    m_mainLayout->addWidget(m_trajectoryGroup);
    m_mainLayout->addWidget(m_qualityGroup);
    m_mainLayout->addWidget(m_loadTemplateButton);
    m_mainLayout->addWidget(m_saveTemplateButton);
    m_mainLayout->addStretch();
    
    // 连接信号
    connect(m_pressureSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParameterPanel::onParameterChanged);
    connect(m_loadTemplateButton, &QPushButton::clicked, this, &ParameterPanel::onLoadTemplate);
    connect(m_saveTemplateButton, &QPushButton::clicked, this, &ParameterPanel::onSaveTemplate);
}

void ParameterPanel::onParameterChanged()
{
    emit parametersChanged();
}

void ParameterPanel::onLoadTemplate()
{
    // 加载参数模板的实现
}

void ParameterPanel::onSaveTemplate()
{
    // 保存参数模板的实现
}

} // namespace UI