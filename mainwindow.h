#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSslSocket>
#include <QPointer>
#include <QTreeWidgetItem>

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
    void listen_newConnection();
    void listen_acceptError(QAbstractSocket::SocketError socketError);

    void on_listenStart_clicked();
    void on_listenStop_clicked();
    void on_forwardStart_clicked();
    void on_forwardStop_clicked();
    void on_requests_itemClicked(QTreeWidgetItem *item, int column);
    void on_sendToClientBtn_clicked();
    void on_sendToServerBtn_clicked();

protected:
    QTreeWidgetItem *addRequestItem(QTcpSocket *socket, const QString &type, QTreeWidgetItem *parentItem = Q_NULLPTR, const QByteArray &extra = QByteArray());
    QTcpSocket *createForwardSocket(QTcpSocket *socket);

private:
    Ui::MainWindow *ui;
    QPointer<QTcpServer> mServer;
    QPointer<QTcpSocket> mSelectedSocket;
    QHash<QTcpSocket*, QPointer<QTcpSocket>> mSocketMaps;
};
#endif // MAINWINDOW_H
