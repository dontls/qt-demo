#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QObject* _textLabel;
    QObject* _sigButton;

private slots:
    void onReceiveQmlButtonClicked();
    void onReceiveQmlButtonProperty(const QVariant &var);
};

#endif // MAINWINDOW_H
