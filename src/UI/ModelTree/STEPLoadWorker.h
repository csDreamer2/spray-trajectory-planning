#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <TopoDS_Shape.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>

/**
 * @brief STEP文件加载工作线程
 * 在后台线程中异步加载STEP文件，避免UI卡顿
 */
class STEPLoadWorker : public QObject {
    Q_OBJECT

public:
    explicit STEPLoadWorker(QObject* parent = nullptr);
    ~STEPLoadWorker();

    /**
     * @brief 加载STEP文件
     * @param filePath 文件路径
     */
    Q_INVOKABLE void loadSTEPFile(const QString& filePath);

signals:
    /**
     * @brief 进度更新信号
     * @param current 当前进度
     * @param total 总进度
     * @param message 消息
     */
    void progressUpdated(int current, int total, const QString& message);

    /**
     * @brief 加载完成信号
     * @param success 是否成功
     * @param message 消息
     * @param actors Actor映射
     * @param shapes 形状映射
     * @param shapeCounter 形状计数
     * @param topLevelName 顶层形状名字
     */
    void loadFinished(bool success, const QString& message,
                      QMap<QString, vtkSmartPointer<vtkActor>> actors,
                      QMap<QString, TopoDS_Shape> shapes,
                      int shapeCounter, const QString& topLevelName);

private:
    void processShape(const TopoDS_Shape& shape, const TDF_Label& label,
                      int& shapeCounter);
    vtkSmartPointer<vtkActor> createActorFromShape(const TopoDS_Shape& shape);

private:
    Handle(TDocStd_Document) m_occDoc;
    QMap<QString, vtkSmartPointer<vtkActor>> m_actorMap;
    QMap<QString, TopoDS_Shape> m_shapeMap;
    QString m_topLevelShapeName;  // 顶层形状的名字
};
