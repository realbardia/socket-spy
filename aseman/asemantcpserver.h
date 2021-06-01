#ifndef ASEMANTCPSERVER_H
#define ASEMANTCPSERVER_H

#include "asemanabstracttcpserver.h"

class AsemanTcpServerPrivate;
class AsemanTcpServer : public AsemanAbstractTcpServer
{
    Q_OBJECT
public:
    AsemanTcpServer(QObject *parent = 0);
    virtual ~AsemanTcpServer();

protected:
    void incomingConnection(qintptr handle);

private:
    AsemanTcpServerPrivate *p;
};

#endif // ASEMANTCPSERVER_H
