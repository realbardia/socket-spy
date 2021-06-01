#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aseman/asemansslserver.h"
#include "aseman/asemantcpserver.h"

#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listen_newConnection_tcp(AsemanAbstractTcpServer::Handle handle)
{
    QTcpSocket *socket;
    if(handle.sslProtocol == QSsl::UnknownProtocol)
    {
        socket = new QTcpSocket(this);
        socket->setSocketDescriptor(handle.handle);
    }
    else
    {
        auto sslSocket = new QSslSocket(this);
        socket = sslSocket;

        connect(sslSocket, &QSslSocket::encrypted, sslSocket, &QSslSocket::readyRead);
        connect(sslSocket, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), this,
              [=](const QList<QSslError> &errors){
            for(const QSslError &err: errors)
                qDebug() << QDateTime::currentDateTime().toString() << err;
        });

        sslSocket->setSocketDescriptor(handle.handle);
        sslSocket->setLocalCertificate(handle.sslLocalCertificate);
        sslSocket->setPrivateKey(handle.sslPrivateKey);
        sslSocket->setProtocol(handle.sslProtocol);
        sslSocket->startServerEncryption();
    }

    auto parentItem = addRequestItem(socket, QString());
    addRequestItem(socket, "CONNECTED", parentItem);

    connect(socket, &QAbstractSocket::readyRead, this, [this, socket, parentItem](){
        addRequestItem(socket, "INCOMING", parentItem);
    });
    connect(socket, &QAbstractSocket::connected, this, [this, socket, parentItem](){
        addRequestItem(socket, "CONNECTED", parentItem);
    });
    connect(socket, &QAbstractSocket::disconnected, this, [this, socket, parentItem](){
        addRequestItem(socket, "DISCONNECTED", parentItem);
        socket->deleteLater();
    });
    connect(socket, &QAbstractSocket::stateChanged, this, [this, socket, parentItem](QAbstractSocket::SocketState state){
        QVariant var = QVariant::fromValue(state);
        var.convert(QVariant::String);

        addRequestItem(socket, "STATE", parentItem, var.toString().toUtf8());
    });
    connect(socket, &QAbstractSocket::errorOccurred, this, [this, socket, parentItem](QAbstractSocket::SocketError err){
        QVariant var = QVariant::fromValue(err);
        var.convert(QVariant::String);

        addRequestItem(socket, "STATE", parentItem, var.toString().toUtf8());
    });

    if(handle.sslProtocol != QSsl::UnknownProtocol)
    {
        auto *ssl = static_cast<QSslSocket*>(socket);
        connect(ssl, &QSslSocket::encrypted, this, [this, socket, parentItem](){
            addRequestItem(socket, "ENCRYPTED", parentItem);
        });
        connect(ssl, &QSslSocket::peerVerifyError, this, [this, socket, parentItem](const QSslError &error){
            QVariant var = QVariant::fromValue(error);
            var.convert(QVariant::String);

            addRequestItem(socket, "SSL_PEER_ERROR", parentItem, var.toString().toUtf8());
        });
        connect(ssl, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors), this, [this, socket, parentItem](const QList<QSslError> &errors){
            for (auto error: errors)
            {
                QVariant var = QVariant::fromValue(error);
                var.convert(QVariant::String);

                addRequestItem(socket, "SSL_PEER_ERROR", parentItem, var.toString().toUtf8());
            }
        });
    }

    mSocketMaps[socket] = createForwardSocket(socket);
}

void MainWindow::listen_newConnection_udp()
{
    qDebug() << "INCOMMING UDP";
    QByteArray datagram;
    while (mUdpServer->hasPendingDatagrams())
    {
        datagram.resize(int(mUdpServer->pendingDatagramSize()));
        mUdpServer->readDatagram(datagram.data(), datagram.size());
        qDebug() << QString("Received IPv4 datagram: \"%1\"").arg(datagram.constData());
    }
}

QTreeWidgetItem *MainWindow::addRequestItem(QAbstractSocket *socket, const QString &type, QTreeWidgetItem *parentItem, const QByteArray &extra)
{
    auto item = new QTreeWidgetItem;
    item->setText(0, socket->peerAddress().toString());
    item->setText(1, QString::number(socket->socketDescriptor()));
    item->setText(2, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    item->setText(3, type);

    mItemSockets[item] = socket;

    if (parentItem)
    {
        auto data = extra.length()? extra : socket->readAll();

        item->setText(4, QString::number(data.size()));
        item->setData(0, Qt::UserRole, data);

        parentItem->insertChild(0, item);
    }
    else
        ui->requests->insertTopLevelItem(0, item);

    return item;
}

QAbstractSocket *MainWindow::createForwardSocket(QAbstractSocket *_s)
{
    if (ui->forwardStart->isEnabled())
        return Q_NULLPTR;

    QPointer<QAbstractSocket> socket = _s;
    QPointer<QAbstractSocket> forward = new QTcpSocket(this);

    connect(socket, &QAbstractSocket::readyRead, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (!forward || !socket)
            return;

        forward->write(socket->readAll());
    });
    connect(socket, &QAbstractSocket::disconnected, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (forward) forward->disconnectFromHost();
        if (socket) socket->deleteLater();
    });

    connect(forward, &QAbstractSocket::readyRead, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (!forward || !socket)
            return;

        socket->write(forward->readAll());
    });
    connect(forward, &QAbstractSocket::disconnected, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (socket) socket->disconnectFromHost();
        if (forward) forward->deleteLater();
    });

    forward->connectToHost(QHostAddress(ui->forwardHost->text()), ui->forwardPort->value());

    return forward;
}

void MainWindow::listen_acceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
}

void MainWindow::on_listenStart_clicked()
{
    if (mTcpServer)
    {
        mTcpServer->close();
        delete mTcpServer;
    }
    if (mUdpServer)
    {
        mUdpServer->close();
        delete mUdpServer;
    }

    auto adrs = ui->listenHost->text();
    if (adrs.isEmpty())
        adrs = QStringLiteral("127.0.0.1");

    switch (ui->listenType->currentIndex())
    {
    case 0: // TCP
    {
        mTcpServer = new AsemanTcpServer(this);

        connect(mTcpServer, &AsemanTcpServer::newConnection, this, &MainWindow::listen_newConnection_tcp);
        connect(mTcpServer, &AsemanTcpServer::acceptError, this, &MainWindow::listen_acceptError);

        mTcpServer->listen(QHostAddress(adrs), ui->listenPort->value());
    }
        break;

    case 1: // SSL
    {
        auto ssl = new AsemanSslServer(this);
        ssl->setSslLocalCertificate(":/keys/server.crt");
        ssl->setSslPrivateKey(":/keys/server.key");
        ssl->setSslProtocol(QSsl::TlsV1_2);

        mTcpServer = ssl;

        connect(mTcpServer, &AsemanSslServer::newConnection, this, &MainWindow::listen_newConnection_tcp);
        connect(mTcpServer, &AsemanSslServer::acceptError, this, &MainWindow::listen_acceptError);

        mTcpServer->listen(QHostAddress(adrs), ui->listenPort->value());
    }
        break;

    case 2: // UDP
    {
        mUdpServer = new QUdpSocket(this);

        connect(mUdpServer, &QUdpSocket::readyRead, this, &MainWindow::listen_newConnection_udp);

        mUdpServer->bind(QHostAddress(adrs), ui->listenPort->value(), QUdpSocket::ShareAddress);
    }
        break;
    }

    ui->listenPort->setEnabled(false);
    ui->listenHost->setEnabled(false);
    ui->listenStart->setEnabled(false);
    ui->listenStop->setEnabled(true);
}

void MainWindow::on_listenStop_clicked()
{
    if (mTcpServer)
    {
        mTcpServer->close();
        delete mTcpServer;
    }

    ui->listenPort->setEnabled(true);
    ui->listenHost->setEnabled(true);
    ui->listenStart->setEnabled(true);
    ui->listenStop->setEnabled(false);
}

void MainWindow::on_forwardStart_clicked()
{
    ui->forwardPort->setEnabled(false);
    ui->forwardHost->setEnabled(false);
    ui->forwardStart->setEnabled(false);
    ui->forwardStop->setEnabled(true);
}

void MainWindow::on_forwardStop_clicked()
{
    ui->forwardPort->setEnabled(true);
    ui->forwardHost->setEnabled(true);
    ui->forwardStart->setEnabled(true);
    ui->forwardStop->setEnabled(false);
}


void MainWindow::on_requests_itemClicked(QTreeWidgetItem *item, int)
{
    auto data = item->data(0, Qt::UserRole).toByteArray();
    ui->details->setText( QString::fromUtf8(data) );
    auto socket = mItemSockets.value(item);

    if (mSocketMaps.contains(socket))
        mSelectedSocket = socket;
    else
        mSelectedSocket = Q_NULLPTR;

    ui->sendGroup->setEnabled(mSelectedSocket);
}


void MainWindow::on_sendToClientBtn_clicked()
{
    if (!mSelectedSocket)
        return;

    mSelectedSocket->write(ui->sendText->toPlainText().toUtf8());
    ui->sendText->clear();
}


void MainWindow::on_sendToServerBtn_clicked()
{
    if (!mSelectedSocket)
        return;

    auto forwardSocket = mSocketMaps.value(mSelectedSocket);
    if (!forwardSocket)
        return;

    forwardSocket->write(ui->sendText->toPlainText().toUtf8());
    ui->sendText->clear();
}

