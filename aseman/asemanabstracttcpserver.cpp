#include "asemanabstracttcpserver.h"

AsemanAbstractTcpServer::AsemanAbstractTcpServer(QObject *parent) : QTcpServer(parent)
{
    qRegisterMetaType<AsemanAbstractTcpServer::Handle>("AsemanAbstractTcpServer::Handle");
}
