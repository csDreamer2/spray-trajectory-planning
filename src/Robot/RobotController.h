#ifndef ROBOTCONTROLLER_H
#define ROBOTCONTROLLER_H

#include <QObject>

namespace Robot {

class RobotController : public QObject
{
    Q_OBJECT

public:
    explicit RobotController(QObject *parent = nullptr);
};

} // namespace Robot

#endif // ROBOTCONTROLLER_H