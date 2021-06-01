#ifndef ASEMANABSTRACTTCPSERVER_H
#define ASEMANABSTRACTTCPSERVER_H

#include <QSslCertificate>
#include <QSslKey>
#include <QTcpServer>

class AsemanAbstractTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    AsemanAbstractTcpServer(QObject *parent = Q_NULLPTR);

    class Handle {
    public:
        Handle() : handle(0), sslProtocol(QSsl::UnknownProtocol) {}
        qintptr handle;
        QSslCertificate sslLocalCertificate;
        QSslKey sslPrivateKey;
        QSsl::SslProtocol sslProtocol;
    };

Q_SIGNALS:
    void newConnection(const Handle &handle);
};

Q_DECLARE_METATYPE(AsemanAbstractTcpServer::Handle)

#endif // ASEMANABSTRACTTCPSERVER_H
