#ifndef MOTOTCPCLIENT_H
#define MOTOTCPCLIENT_H

#include <QObject>

namespace Robot {

class MotoTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit MotoTcpClient(QObject *parent = nullptr);
};

} // namespace Robot

#endif // MOTOTCPCLIENT_H