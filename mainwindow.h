#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSslSocket>
#include <QPointer>
#include <QTreeWidgetItem>

#include "aseman/asemanabstracttcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void listen_newConnection_tcp(AsemanAbstractTcpServer::Handle handle);
    void listen_newConnection_udp();
    void listen_acceptError(QAbstractSocket::SocketError socketError);

    void on_listenStart_clicked();
    void on_listenStop_clicked();
    void on_forwardStart_clicked();
    void on_forwardStop_clicked();
    void on_requests_itemClicked(QTreeWidgetItem *item, int column);
    void on_sendToClientBtn_clicked();
    void on_sendToServerBtn_clicked();
    void on_saveBtn_clicked();
    void on_restoreBtn_clicked();
    void on_clearBtn_clicked();
    void on_requests_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

protected:
    QTreeWidgetItem *addRequestItem(QAbstractSocket *socket, const QString &type, QTreeWidgetItem *parentItem = Q_NULLPTR, const QByteArray &extra = QByteArray(), QByteArray *dataRes = Q_NULLPTR);
    QAbstractSocket *createForwardSocket(QAbstractSocket *socket, QTreeWidgetItem *parentItem);

    QVariantList saveData(QTreeWidgetItem *item = Q_NULLPTR);
    void restoreData(const QVariantList list, QTreeWidgetItem *item = Q_NULLPTR);

private:
    Ui::MainWindow *ui;
    QPointer<AsemanAbstractTcpServer> mTcpServer;
    QPointer<QUdpSocket> mUdpServer;
    QPointer<QAbstractSocket> mSelectedSocket;
    QHash<QAbstractSocket*, QPointer<QAbstractSocket>> mSocketMaps;
    QHash<QTreeWidgetItem*, QPointer<QAbstractSocket>> mItemSockets;
};
#endif // MAINWINDOW_H
