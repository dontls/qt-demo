#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtQml>
#include <QQuickItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject* root = (QObject*)ui->quickWidget->rootObject();
    QObject* qmlButton = root->findChild<QObject*>("qmlButton");

    if(qmlButton != NULL) {
        connect(qmlButton, SIGNAL(clicked()), this, SLOT(onReceiveQmlButtonClicked()));
    }

    _textLabel = root->findChild<QObject*>("objNameL");

    _sigButton = root->findChild<QObject*>("objNameB");
    if (_sigButton)
    {
        //When a QML object type is used as a signal parameter, the parameter should use var as the type,
        //and the value should be received in C++ using the QVariant type
        QObject::connect(_sigButton, SIGNAL(qmlSignal(QVariant)), this, SLOT(onReceiveQmlButtonProperty(QVariant)));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onReceiveQmlButtonClicked()
{
    qDebug() << __FUNCTION__;
    //获得qml控件属性值
    qDebug() << QQmlProperty(_textLabel, "text").read().toString();
    if (_textLabel) {

        // modify the item's properties using QObject::setProperty() or QQmlProperty:
        QQmlProperty(_textLabel, "color").write("#888888");
        QQmlProperty(_textLabel, "text").write("Hello");
    }
}

void MainWindow::onReceiveQmlButtonProperty(const QVariant &var)
{
    qDebug() << __FUNCTION__ << var;
    QQuickItem *item =qobject_cast<QQuickItem*>(var.value<QObject*>());
    qDebug() << "Item dimensions:" << item->width() << item->height();

    if(_textLabel) {
        //c++ 直接调用qml函数 反射
        QMetaObject::invokeMethod(_textLabel, "setLtext");
        //QQmlProperty(_textLabel, "color").write("#9a2cf2");
        QMetaObject::invokeMethod(_textLabel, "setLColor", Q_ARG(QVariant, "#9a2cf2"), Q_ARG(QVariant, 16));
    }
}
