#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <QObject>

namespace Data {

class DataModels : public QObject
{
    Q_OBJECT

public:
    explicit DataModels(QObject *parent = nullptr);
};

} // namespace Data

#endif // DATAMODELS_H