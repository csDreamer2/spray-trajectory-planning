#ifndef COLLISIONDETECTOR_H
#define COLLISIONDETECTOR_H

#include <QObject>

namespace Simulation {

class CollisionDetector : public QObject
{
    Q_OBJECT

public:
    explicit CollisionDetector(QObject *parent = nullptr);
};

} // namespace Simulation

#endif // COLLISIONDETECTOR_H