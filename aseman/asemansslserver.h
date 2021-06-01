#ifndef ASEMANSSLSERVER_H
#define ASEMANSSLSERVER_H

#include "SslServer.h"

class AsemanSslServerPrivate;
class AsemanSslServer : public SslServer
{
    Q_OBJECT
public:
    AsemanSslServer(QObject *parent = 0);
    virtual ~AsemanSslServer();

private:
    AsemanSslServerPrivate *p;
};

#endif // ASEMANSSLSERVER_H
