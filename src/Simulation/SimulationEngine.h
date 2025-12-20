#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <QObject>

namespace Simulation {

class SimulationEngine : public QObject
{
    Q_OBJECT

public:
    explicit SimulationEngine(QObject *parent = nullptr);
};

} // namespace Simulation

#endif // SIMULATIONENGINE_H