#ifndef TRAJECTORYPLANNER_H
#define TRAJECTORYPLANNER_H

#include <QObject>

namespace Data {

class TrajectoryPlanner : public QObject
{
    Q_OBJECT

public:
    explicit TrajectoryPlanner(QObject *parent = nullptr);
};

} // namespace Data

#endif // TRAJECTORYPLANNER_H