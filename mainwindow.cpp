#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFuture> // for async computation
#include <QtConcurrent/QtConcurrent> // for async computation

#include "Analyzer.h"
#include "result.h"

#include <iostream>
#include <stdlib.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_analyzer(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateConsole(const QString &text)
{
    ui->textBrowserOutput->append(text);
}

void MainWindow::setProgressBar(int val)
{
    ui->progressBar->setValue(val);
}

void MainWindow::CreateAlert(const QString &text)
{
    QMessageBox::information(this, "Alert", text);
}

void MainWindow::RetrieveDirectories(const QString &text)
{
    QDir dir(text);

    if (!dir.exists()) {
        qDebug() << "Directory does not exist: " << text;
        return;
    }

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo& fileInfo, fileInfoList) {
        QString directoryPath = fileInfo.absoluteFilePath();
        allDirectories.append(directoryPath); // Store directory path in the list
        RetrieveDirectories(directoryPath); // Recursively call to traverse subdirectories
    }
}


void MainWindow::on_pushButtonSaveFeedback_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Feedback", QDir::homePath(), tr("Text files (*.txt);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        QTextStream out(&file);
        QString text = ui->textEdit->toPlainText();
        out << text;
        file.flush();
        file.close();
    }
}

void MainWindow::on_pushButtonAnalyze_clicked(const QString& directoryPath)
{
    /* if analyzer is still running, wait for it to finish */
    if (m_analyzer && !m_analyzer->status_done) {
        QMessageBox::information(this, tr("Error"),
                                 tr("Please wait for the program to finish"));
        return;
    }

    /* recreate analyzer object */
    ui->textBrowserOutput->clear();
    if (ui->checkBoxDivision->isChecked()) {
        param.divisionEnabled = true;
    } else {
        param.divisionEnabled = false;
    }

    delete m_analyzer;
    m_analyzer = new Analyzer(param);
    QObject::connect(m_analyzer,&Analyzer::msgToConsole,this,&MainWindow::updateConsole);
    QObject::connect(m_analyzer, &Analyzer::updateProgressBar, this, &MainWindow::setProgressBar);
    QObject::connect(m_analyzer, &Analyzer::alertToWindow, this, &MainWindow::CreateAlert);

    QtConcurrent::run(this->m_analyzer, &Analyzer::analyze);
    while( !m_analyzer->status_done )
    {
        QThread::sleep(1);
    }
    AutoExport(directoryPath);
}

/*new -------------------- */

//void MainWindow::on_pushButtonOpenDirectory_clicked()
//{
//    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath(), QFileDialog::ShowDirsOnly
//                                                    | QFileDialog::DontResolveSymlinks);
//    QDir dir(dirPath);

//    QList <QFileInfo> fileList= dir.entryInfoList();

//    if(ui->listWidget->count() != 0)
//    {
//        ui->listWidget->clear();
//    }

//    for (int i=0; i< fileList.size(); i++)
//    {
//        ui->listWidget->addItem(fileList.at(i).fileName());
//    }
//     connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(doSomething(QListWidgetItem*)));
//}

/*new -------------------- */





void MainWindow::on_pushButtonStudentFolder_clicked()
{
    QString text = QFileDialog::getExistingDirectory(this, "Open Student Folder", QDir::homePath(), QFileDialog::ShowDirsOnly
                                                                                                       | QFileDialog::DontResolveSymlinks);
    if (text != "")
    {
        ui->lineEditStudentFolder->setText(text);
        allDirectories.clear();
        //RetrieveDirectories(dir);
        QDir dir(text);
        QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach (const QFileInfo& fileInfo, fileInfoList)
        {
            QString directoryPath = fileInfo.absoluteFilePath();
            string base = directoryPath.toStdString();
            param.studentModel = base + "/model.off";
            param.studentCenterPoint = base + "/center_point.pp";
            param.studentMidpoint =  base + "/mid_point.pp";
            param.studentMarginPoints =  base + "/margin_points.pp";
            param.studentAxialPoints = base + "/axial_points.pp";
            param.studentOcclusalPoints = base + "/occlusal_points.pp";
            param.studentGingivaPoints = base + "/gingiva_points.pp";
            param.originalModel = base + "/original_model.off";
            ui->lineEditStudentModel->setText(QString::fromStdString(param.studentModel));
            ui->lineEditStudentCenter->setText(QString::fromStdString(param.studentCenterPoint));
            ui->lineEditStudentMidpoint->setText(QString::fromStdString(param.studentMidpoint));
            ui->lineEditStudentMarginPoints->setText(QString::fromStdString(param.studentMarginPoints));
            ui->lineEditStudentAxialPoints->setText(QString::fromStdString(param.studentAxialPoints));
            ui->lineEditStudentOcclusalPoints->setText(QString::fromStdString(param.studentOcclusalPoints));
            ui->lineEditStudentGingivaPoints->setText(QString::fromStdString(param.studentGingivaPoints));
            ui->lineEditOriginalModel->setText(QString::fromStdString(param.originalModel));
            on_pushButtonAnalyze_clicked(directoryPath);
        }
    }
}

void MainWindow::on_pushButtonStudentModel_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Model", QDir::homePath());
    if (filename != "") {
        param.studentModel = filename.toStdString();
        ui->lineEditStudentModel->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentCenter_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Model Center", QDir::homePath());
    if (filename != "") {
        param.studentCenterPoint = filename.toStdString();
        ui->lineEditStudentCenter->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentMidpoint_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Model Midpoint", QDir::homePath());
    if (filename != "") {
        param.studentMidpoint = filename.toStdString();
        ui->lineEditStudentMidpoint->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentMarginPoints_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Margin Points", QDir::homePath());
    if (filename != "") {
        param.studentMarginPoints = filename.toStdString();
        ui->lineEditStudentMarginPoints->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentAxialPoints_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Axial Points", QDir::homePath());
    if (filename != "") {
        param.studentAxialPoints = filename.toStdString();
        ui->lineEditStudentAxialPoints->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentOcclusalPoints_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Occlusal Points", QDir::homePath());
    if (filename != "") {
        param.studentOcclusalPoints = filename.toStdString();
        ui->lineEditStudentOcclusalPoints->setText(filename);
    }
}

void MainWindow::on_pushButtonStudentGingivaPoints_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Student Gingiva Points", QDir::homePath());
    if (filename != "") {
        param.studentGingivaPoints = filename.toStdString();
        ui->lineEditStudentGingivaPoints->setText(filename);
    }
}


void MainWindow::on_pushButtonOriginalModel_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Original Model", QDir::homePath());
    if (filename != "") {
        param.originalModel = filename.toStdString();
        ui->lineEditOriginalModel->setText(filename);
    }
}

void MainWindow::on_checkBoxDivision_toggled(bool checked)
{
    if (checked) {
        ui->labelStudentCenter->setEnabled(true);
        ui->labelStudentMidpoint->setEnabled(true);
        ui->lineEditStudentCenter->setEnabled(true);
        ui->lineEditStudentMidpoint->setEnabled(true);
        ui->pushButtonStudentCenter->setEnabled(true);
        ui->pushButtonStudentMidpoint->setEnabled(true);
    } else {
        ui->labelStudentCenter->setDisabled(true);
        ui->labelStudentMidpoint->setDisabled(true);
        ui->lineEditStudentCenter->setDisabled(true);
        ui->lineEditStudentMidpoint->setDisabled(true);
        ui->pushButtonStudentCenter->setDisabled(true);
        ui->pushButtonStudentMidpoint->setDisabled(true);
    }
}

void MainWindow::on_pushButtonExport_clicked()
{
    if (!m_analyzer || !m_analyzer->status_done) {
        QMessageBox::information(this, tr("Export Error"),
                                 tr("Results not available"));
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results To", QDir::homePath(), tr("csv files (*.csv);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }
    QTextStream out(&file);

    /* student name */
    out << "Name: " << ui->lineEditStudentName->text() << ",";

    /* stats headers */
    out << "Max," << "Min," << "Avg," << "\n";

    /* metrics */
    if (m_analyzer->param.divisionEnabled) {
        QString region_map[4] = {"Lingual", "Buccal", "Mesial", "Distal"};
        for (int i = 0; i < 4; i++) {
            out << "Shoulder Width (" << region_map[i] << "),";
            out << m_analyzer->student_result.shoulder_width_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Taper (" << region_map[i] << "),";
            out << m_analyzer->student_result.taper_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Axial Wall Height (" << region_map[i] << "),";
            out << m_analyzer->student_result.axial_wall_height_stats[i].to_csv();
        }
        //    for (int i = 0; i < 4; i++) {
        //      out << "Margin Depth (" << region_map[i] << "),";
        //      out << m_analyzer->student_result.margin_depth_stats[i].to_csv();
        //    }
        for (int i = 0; i < 4; i++) {
            out << "Occlusal Reduction (" << region_map[i] << "),";
            out << m_analyzer->student_result.occlusal_reduction_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Gingival Extension (" << region_map[i] << "),";
            out << m_analyzer->student_result.gingival_extension_stats[i].to_csv();
        }
    } else {
        out << "Shoulder Width,";
        out << m_analyzer->student_result.shoulder_width_stats[0].to_csv();
        out << "Taper,";
        out << m_analyzer->student_result.taper_stats[0].to_csv();
        out << "Axial Wall Height,";
        out << m_analyzer->student_result.axial_wall_height_stats[0].to_csv();
        out << "Margin Depth,";
        // out << m_analyzer->student_result.margin_depth_stats[0].to_csv();
        out << "Occlusal Reduction,";
        out << m_analyzer->student_result.occlusal_reduction_stats[0].to_csv();
        out << "Gingival Extension,";
        out << m_analyzer->student_result.gingival_extension_stats[0].to_csv();
    }

    /* data points */
    out << "\n";
    out << "Metric: Shoulder Width\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.shoulder_width_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.shoulder_width_data);

    out << "Metric: Taper\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.taper_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.taper_data);

    out << "Metric: Axial Wall Height\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.axial_wall_height_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.axial_wall_height_data);

    //  out << "Metric: Margin Depth\n";
    //  out << "Point,";
    //  out << points_to_csv(m_analyzer->student_result.margin_depth_data);
    //  out << "Value,";
    //  out << values_to_csv(m_analyzer->student_result.margin_depth_data);

    out << "Metric: Occlusal Reduction\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.occlusal_reduction_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.occlusal_reduction_data);

    out << "Metric: Gingival Extension\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.gingival_extension_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.gingival_extension_data);

    file.flush();
    file.close();

    QMessageBox::information(this, tr("Success"),
                             tr("Result Exported to ").append(fileName));
}

void MainWindow::AutoExport(const QString& fileName)
{
    if (!m_analyzer || !m_analyzer->status_done) {
        QMessageBox::information(this, tr("Export Error"),
                                 tr("Results not available"));
        return;
    }

    QString finalPath = fileName + "/export.csv";

    QFile file(finalPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Unable to open file"),
                                 file.errorString());
        return;
    }
    QTextStream out(&file);

    /* student name */
    out << "Name: " << ui->lineEditStudentName->text() << ",";

    /* stats headers */
    out << "Max," << "Min," << "Avg," << "\n";

    /* metrics */
    if (m_analyzer->param.divisionEnabled) {
        QString region_map[4] = {"Lingual", "Buccal", "Mesial", "Distal"};
        for (int i = 0; i < 4; i++) {
            out << "Shoulder Width (" << region_map[i] << "),";
            out << m_analyzer->student_result.shoulder_width_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Taper (" << region_map[i] << "),";
            out << m_analyzer->student_result.taper_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Axial Wall Height (" << region_map[i] << "),";
            out << m_analyzer->student_result.axial_wall_height_stats[i].to_csv();
        }
        //    for (int i = 0; i < 4; i++) {
        //      out << "Margin Depth (" << region_map[i] << "),";
        //      out << m_analyzer->student_result.margin_depth_stats[i].to_csv();
        //    }
        for (int i = 0; i < 4; i++) {
            out << "Occlusal Reduction (" << region_map[i] << "),";
            out << m_analyzer->student_result.occlusal_reduction_stats[i].to_csv();
        }
        for (int i = 0; i < 4; i++) {
            out << "Gingival Extension (" << region_map[i] << "),";
            out << m_analyzer->student_result.gingival_extension_stats[i].to_csv();
        }
    } else {
        out << "Shoulder Width,";
        out << m_analyzer->student_result.shoulder_width_stats[0].to_csv();
        out << "Taper,";
        out << m_analyzer->student_result.taper_stats[0].to_csv();
        out << "Axial Wall Height,";
        out << m_analyzer->student_result.axial_wall_height_stats[0].to_csv();
        out << "Margin Depth,";
        // out << m_analyzer->student_result.margin_depth_stats[0].to_csv();
        out << "Occlusal Reduction,";
        out << m_analyzer->student_result.occlusal_reduction_stats[0].to_csv();
        out << "Gingival Extension,";
        out << m_analyzer->student_result.gingival_extension_stats[0].to_csv();
    }

    /* data points */
    out << "\n";
    out << "Metric: Shoulder Width\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.shoulder_width_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.shoulder_width_data);

    out << "Metric: Taper\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.taper_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.taper_data);

    out << "Metric: Axial Wall Height\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.axial_wall_height_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.axial_wall_height_data);

    //  out << "Metric: Margin Depth\n";
    //  out << "Point,";
    //  out << points_to_csv(m_analyzer->student_result.margin_depth_data);
    //  out << "Value,";
    //  out << values_to_csv(m_analyzer->student_result.margin_depth_data);

    out << "Metric: Occlusal Reduction\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.occlusal_reduction_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.occlusal_reduction_data);

    out << "Metric: Gingival Extension\n";
    out << "Point,";
    out << points_to_csv(m_analyzer->student_result.gingival_extension_data);
    out << "Value,";
    out << values_to_csv(m_analyzer->student_result.gingival_extension_data);

    file.flush();
    file.close();

    // //QMessageBox::information(this, tr("Success"),
    //                          tr("Result Exported to ").append(fileName));
}
