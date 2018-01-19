#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QString>
#include <QDebug>
#include <QImage>
#include <QThread>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSlider>
#include <QWidget>
#include <QPoint>
#include <QSpinBox>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "ocvhelpers.h"
#include "filters.h"

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
    void openFile();
    void saveFile();
    void saveAsFile();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();

    void toGrayscale();
    void toColorize();
    void kelvinEffect();
    void coolEffect();
    void mosaicEffect();
    void resetImageOriginal();

//    void displayCoords(const QPoint& pos);

    void setImageL(const QImage& dst);

    void on_vignetteAct_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QAction* fileOpenAct;
    QAction* fileSaveAct;
    QAction* fileSaveAsAct;
    QAction* exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QAction *grayscaleAct;
    QAction* colorizeAct;
    QAction* kelvinAct;
    QAction* coolAct;
    QAction* originalAct;
    QAction* vignetteAct;
    QAction* mosaicAct;
    QSpinBox *spinBox;


    int gridSizeval;
    cv::Mat img;
    cv::Mat colorImg;
    cv::Mat originalCopy;

    QString oFilename;

    double scaleFactor;

    void setupSS();
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

};

#endif // MAINWINDOW_H
