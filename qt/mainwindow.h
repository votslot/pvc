#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionTest_1M_3_triggered();

    void on_actionLoadLas_triggered();

    void on_actiontestme_triggered();

    void on_actiontest123_triggered();

    void on_openGLWidget_destroyed(QObject *arg1);

signals:
    void test_run_signal();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
