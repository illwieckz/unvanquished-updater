#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "downloadworker.h"
#include "aria2/src/includes/aria2/aria2.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    totalSize(0),
    paused(false)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(startUpdate()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startUpdate(void)
{
    ui->textBrowser->append("Starting update\n");
    ui->toggleButton->setEnabled(true);
    ui->updateButton->setEnabled(false);
    worker = new DownloadWorker();
    worker->addUri("http://cdn.unvanquished.net/current.torrent");
    worker->moveToThread(&thread);
    connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(onDownloadEvent(int)), this, SLOT(onDownloadEvent(int)));
    connect(worker, SIGNAL(downloadSpeedChanged(int)), this, SLOT(setDownloadSpeed(int)));
    connect(worker, SIGNAL(uploadSpeedChanged(int)), this, SLOT(setUploadSpeed(int)));
    connect(worker, SIGNAL(totalSizeChanged(int)), this, SLOT(setTotalSize(int)));
    connect(worker, SIGNAL(completedSizeChanged(int)), this, SLOT(setCompletedSize(int)));
    connect(&thread, SIGNAL(started()), worker, SLOT(download()));
    connect(ui->toggleButton, SIGNAL(clicked()), this, SLOT(toggleDownload()));
    thread.start();
}

void MainWindow::toggleDownload(void)
{
    worker->toggle();
    paused = !paused;
    if (paused) {
        ui->toggleButton->setText("Resume");
    } else {
        ui->toggleButton->setText("Pause");
    }
}


void MainWindow::setDownloadSpeed(int speed)
{
    ui->downloadSpeed->setText(sizeToString(speed) + "/s");
}

void MainWindow::setUploadSpeed(int speed)
{
    ui->uploadSpeed->setText(sizeToString(speed) + "/s");
}

void MainWindow::setTotalSize(int size)
{
    ui->totalSize->setText(sizeToString(size));
    totalSize = size;
}

void MainWindow::setCompletedSize(int size)
{
    ui->completedSize->setText(sizeToString(size));
    ui->progressBar->setValue((static_cast<float>(size) / totalSize) * 100);
}

void MainWindow::onDownloadEvent(int event)
{
    switch (event) {
        case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
            ui->textBrowser->append("yay\n");
            thread.quit();
            thread.wait();
            break;

        case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
            ui->textBrowser->append("Torrent downloaded\n");
            break;

        case aria2::EVENT_ON_DOWNLOAD_ERROR:
            ui->textBrowser->append("Error received while downloading\n");
            break;

        case aria2::EVENT_ON_DOWNLOAD_PAUSE:
            ui->textBrowser->append("Download paused\n");
            break;

        case aria2::EVENT_ON_DOWNLOAD_START:
            ui->textBrowser->append("Download started\n");
            break;

        case aria2::EVENT_ON_DOWNLOAD_STOP:
            ui->textBrowser->append("Download stopped\n");
            break;
    }
}

QString MainWindow::sizeToString(int size)
{
    static QString sizes[] = { "Bytes", "KiB", "MiB", "GiB" };
    const int num_sizes = 4;
    int i = 0;
    while (size > 1024 && i++ < 4) {
        size /= 1024;
    }

    return QString::number(size) + " " + sizes[std::min(i, num_sizes - 1)];
}
