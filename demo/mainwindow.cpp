/******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 https://github.com/labyrinthofdreams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
******************************************************************************/

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