#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
extern void RunQuad();
extern void ShowQuad();
extern int main123(int argc, char **argv);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    //QFileDialog::getOpenFileNames(this, tr("Open File"),"/path/to/file/",tr("Las Files (*.las)"));
    ShowQuad();

}
