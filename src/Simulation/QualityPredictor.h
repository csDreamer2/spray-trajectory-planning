#ifndef QUALITYPREDICTOR_H
#define QUALITYPREDICTOR_H

#include <QObject>

namespace Simulation {

class QualityPredictor : public QObject
{
    Q_OBJECT

public:
    explicit QualityPredictor(QObject *parent = nullptr);
};

} // namespace Simulation

#endif // QUALITYPREDICTOR_H