#include "asemantcpserver.h"

class AsemanTcpServerPrivate
{
public:
};

AsemanTcpServer::AsemanTcpServer(QObject *parent) :
    AsemanAbstractTcpServer(parent)
{
    p = new AsemanTcpServerPrivate;
}

void AsemanTcpServer::incomingConnection(qintptr handle)
{
    Handle h;
    h.handle = handle;

    Q_EMIT newConnection(h);
}

AsemanTcpServer::~AsemanTcpServer()
{
    delete p;
}

