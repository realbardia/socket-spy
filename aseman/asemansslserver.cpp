#include "asemansslserver.h"

class AsemanSslServerPrivate
{
public:
};

AsemanSslServer::AsemanSslServer(QObject *parent) :
    SslServer(parent)
{
    p = new AsemanSslServerPrivate;
}

AsemanSslServer::~AsemanSslServer()
{
    delete p;
}

