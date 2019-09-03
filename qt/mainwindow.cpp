#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //connect(this, SIGNAL(test_run_signal()), ui->openGLWidget, SLOT(testRun()));
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionTest_1M_3_triggered()
{
    emit test_run_signal();
}

void MainWindow::on_actionLoadLas_triggered()
{

}

void MainWindow::on_actiontestme_triggered()
{

}

void MainWindow::on_actiontest123_triggered()
{

}

void MainWindow::on_openGLWidget_destroyed(QObject *arg1)
{

}
