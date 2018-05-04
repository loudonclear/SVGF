#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "view.h"
#include <QGLFormat>
#include <QFileDialog>
#include <iostream>

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

void MainWindow::file_open() {
    QString file = QFileDialog::getOpenFileName(this, "Open File", "./res/scenes/");
    if (!file.isNull()) {
        ui->view->loadScene(file);
        change_settings();
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Save File", "./res/scenes/");
    if (!file.isNull()) {
        ui->view->grabFrameBuffer().save(file);
    }
}

void MainWindow::change_settings() {

    int rendermode = ui->fullButton->isChecked() ? 0 : (ui->directButton->isChecked() ? 1 : (ui->indirectButton->isChecked() ? 2 : 3));
    int samples = ui->samplesSlider->value();
    float temporalAlpha = ui->temporalSlider->value() / 100.f;
    bool temporalReprojection = ui->temporalReprojectionCheckBox->isChecked();
    int waveletIterations = ui->waveletSlider->value();
    bool fxaa = ui->fxaaCheckBox->isChecked();


    ui->samplesSliderBox->setValue(samples);
    ui->waveletSliderBox->setValue(waveletIterations);
    ui->temporalSliderBox->setValue(temporalAlpha);

    float sigmaP = ui->sigmaPBox->value();
    float sigmaN = ui->sigmaNBox->value();
    float sigmaL = ui->sigmaLBox->value();

    ui->view->change_settings(rendermode, samples, temporalAlpha, temporalReprojection, waveletIterations, sigmaP, sigmaN, sigmaL, fxaa);
}
