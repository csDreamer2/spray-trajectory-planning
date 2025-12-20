#ifndef POINTCLOUDPROCESSOR_H
#define POINTCLOUDPROCESSOR_H

#include <QObject>

namespace Data {

class PointCloudProcessor : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudProcessor(QObject *parent = nullptr);
};

} // namespace Data

#endif // POINTCLOUDPROCESSOR_H