#ifndef PROGRAMGENERATOR_H
#define PROGRAMGENERATOR_H

#include <QObject>

namespace Robot {

class ProgramGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ProgramGenerator(QObject *parent = nullptr);
};

} // namespace Robot

#endif // PROGRAMGENERATOR_H