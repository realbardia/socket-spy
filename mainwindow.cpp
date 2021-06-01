#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::listen_newConnection()
{
    auto socket = mServer->nextPendingConnection();
    auto parentItem = addRequestItem(socket, QString());
    addRequestItem(socket, "CONNECTED", parentItem);

    connect(socket, &QTcpSocket::readyRead, this, [this, socket, parentItem](){
        addRequestItem(socket, "INCOMING", parentItem);
    });
    connect(socket, &QTcpSocket::connected, this, [this, socket, parentItem](){
        addRequestItem(socket, "CONNECTED", parentItem);
    });
    connect(socket, &QTcpSocket::disconnected, this, [this, socket, parentItem](){
        addRequestItem(socket, "DISCONNECTED", parentItem);
        socket->deleteLater();
    });
    connect(socket, &QTcpSocket::stateChanged, this, [this, socket, parentItem](QAbstractSocket::SocketState state){
        QVariant var = QVariant::fromValue(state);
        var.convert(QVariant::String);

        addRequestItem(socket, "STATE", parentItem, var.toString().toUtf8());
    });
    connect(socket, &QTcpSocket::errorOccurred, this, [this, socket, parentItem](QAbstractSocket::SocketError err){
        QVariant var = QVariant::fromValue(err);
        var.convert(QVariant::String);

        addRequestItem(socket, "STATE", parentItem, var.toString().toUtf8());
    });

    mSocketMaps[socket] = createForwardSocket(socket);
}

QTreeWidgetItem *MainWindow::addRequestItem(QTcpSocket *socket, const QString &type, QTreeWidgetItem *parentItem, const QByteArray &extra)
{
    auto item = new QTreeWidgetItem;
    item->setText(0, socket->peerAddress().toString());
    item->setText(1, QString::number(socket->socketDescriptor()));
    item->setText(2, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    item->setText(3, type);
    item->setData(0, Qt::UserRole+1, QVariant::fromValue(socket));

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

QTcpSocket *MainWindow::createForwardSocket(QTcpSocket *_s)
{
    if (ui->forwardStart->isEnabled())
        return Q_NULLPTR;

    QPointer<QTcpSocket> socket = _s;
    QPointer<QTcpSocket> forward = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (!forward || !socket)
            return;

        forward->write(socket->readAll());
    });
    connect(socket, &QTcpSocket::disconnected, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (forward) forward->disconnectFromHost();
        if (socket) socket->deleteLater();
    });

    connect(forward, &QTcpSocket::readyRead, this, [this, socket, forward](){
        if (ui->forwardStart->isEnabled()) return;
        if (!forward || !socket)
            return;

        socket->write(forward->readAll());
    });
    connect(forward, &QTcpSocket::disconnected, this, [this, socket, forward](){
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
    if (mServer)
    {
        mServer->close();
        delete mServer;
    }

    mServer = new QTcpServer(this);

    connect(mServer, &QTcpServer::newConnection, this, &MainWindow::listen_newConnection);
    connect(mServer, &QTcpServer::acceptError, this, &MainWindow::listen_acceptError);

    auto adrs = ui->listenHost->text();
    if (adrs.isEmpty())
        adrs = QStringLiteral("127.0.0.1");

    mServer->listen(QHostAddress(adrs), ui->listenPort->value());

    ui->listenPort->setEnabled(false);
    ui->listenHost->setEnabled(false);
    ui->listenStart->setEnabled(false);
    ui->listenStop->setEnabled(true);
}

void MainWindow::on_listenStop_clicked()
{
    if (mServer)
    {
        mServer->close();
        delete mServer;
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
    auto socket = item->data(0, Qt::UserRole+1).value<QTcpSocket*>();

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

