#include <QFileDialog>
#include <QIcon>
#include <QImage>
#include <QList>
#include <QListWidgetItem>
#include <QSize>
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->spinBox->setValue(10);

    const auto list = QFileDialog::getOpenFileNames();
    if(list.isEmpty()) {
        return;
    }

    QList<QListWidgetItem*> items;
    for(const auto &item : list) {
        auto w = new QListWidgetItem;
        QIcon icon;
        icon.addFile(item);
        w->setIcon(icon);
        ui->listWidget->insertItem(0, w);
    }

    ui->listWidget->setResizeMode(QListView::Adjust);
    QImage image;
    image.load(list.first());
    ui->listWidget->setIconSize(image.scaledToWidth(150).size());
    ui->listWidget->setFixedWidth(180);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_spinBox_valueChanged(const int arg1)
{
    ui->widget->setSpacing(arg1);
}
