#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QScroller>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

MainWindow::~MainWindow()
{
    delete ui;
}
