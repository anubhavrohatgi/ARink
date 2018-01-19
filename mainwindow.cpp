#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gridSizeval =8;


    spinBox = new QSpinBox(this);
    ui->mainToolBar->addWidget(spinBox);

    fileOpenAct = ui->fileOpenAct;
    fileSaveAsAct = ui->saveAsAct;
    fileSaveAct = ui->saveImgAct;
    exitAct = ui->actionExit;
    zoomInAct = ui->zoomInAct;
    zoomOutAct = ui->zoomOutAct;
    normalSizeAct = ui->normalSizeAct;
    fitToWindowAct = ui->fitToWindowAct;
    grayscaleAct = ui->grayscaleAct;
    colorizeAct = ui->colorizeAct;
    kelvinAct = ui->kelvinAct;
    coolAct = ui->coolAct;
    originalAct = ui->originalAct;
    vignetteAct = ui->vignetteAct;
    mosaicAct = ui->mosaicAct;


    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);


    QImage temp(":/frontend/resources/test1.jpg");
//    imageLabel->setPixmap(QPixmap::fromImage(temp));
    imageLabel->setStyleSheet("background-image: url(:/frontend/resources/test1.jpg);");
    imageLabel->resize(this->size());

    scrollArea = new QScrollArea;
//    scrollArea->setGeometry(100,20,this->width()-120,this->height()-100);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);
    resize(500, 400);

    setupSS();
}

void MainWindow::setupSS()
{
    spinBox->setValue(gridSizeval);
    connect(fileOpenAct,SIGNAL(triggered()),this,SLOT(openFile()));
    connect(fileSaveAct,SIGNAL(triggered()),this,SLOT(saveFile()));
    connect(fileSaveAsAct,SIGNAL(triggered()),this,SLOT(saveAsFile()));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));
    connect(grayscaleAct, SIGNAL(triggered()), this, SLOT(toGrayscale()));
    connect(colorizeAct, SIGNAL(triggered()), this, SLOT(toColorize()));
    connect(kelvinAct, SIGNAL(triggered()), this, SLOT(kelvinEffect()));
    connect(coolAct, SIGNAL(triggered()), this, SLOT(coolEffect()));
    connect(originalAct, SIGNAL(triggered()), this, SLOT(resetImageOriginal()));
    connect(vignetteAct, SIGNAL(triggered()), this, SLOT(on_vignetteAct_triggered()));
    connect(mosaicAct, SIGNAL(triggered()), this, SLOT(mosaicEffect()));


//    connect(this, SIGNAL(Pos()), this, SLOT(on_vignetteAct_triggered()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}


void MainWindow::scaleImage(double factor)
{
//    Q_ASSERT(imageLabel->pixmap());
    if(imageLabel->pixmap() == nullptr)
        return;
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}


void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}


void MainWindow::zoomIn()
{
    scaleImage(1.25);
}

void MainWindow::zoomOut()
{
    scaleImage(0.8);
}

void MainWindow::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void MainWindow::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                     tr("Open File"), QDir::homePath()+"/Pictures");
    if (!fileName.isEmpty()) {
         QImage* image = new QImage(fileName);
         if (image->isNull()) {
             QMessageBox::warning(this, tr("ARink"),
                                      tr("Cannot load %1.").arg(fileName));
             return;
         }
         imageLabel->setPixmap(QPixmap::fromImage(*image));
         scaleFactor = 1.0;

         fitToWindowAct->setEnabled(true);
         updateActions();

         if (!fitToWindowAct->isChecked())
             imageLabel->adjustSize();


         img = QImageToCvMat(*image,true);


         if(img.channels() >3){
             colorImg = img.clone();
         }

         originalCopy = img.clone();

         delete image;

         oFilename = fileName;

    }
}

void MainWindow::saveAsFile()
{
    if(img.empty())
        return;
    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save Image As"), "",
           tr("Image (*.png);;(*.jpg);;(*.bmp)"));
    qDebug()<<fileName;


    bool state = cv::imwrite(fileName.toStdString(),img);

    if(state){
        QMessageBox::information(this, tr("ARink"),
                                 tr("File Saved %1.").arg(fileName));
    } else {
        QMessageBox::warning(this, tr("ARink"),
                                 tr("Cannot save %1.").arg(fileName));
    }
}

void MainWindow::saveFile()
{
    if(img.empty() || oFilename.length() < 1 )
        return;

    qDebug()<<oFilename;


    bool state = cv::imwrite(oFilename.toStdString(),img);

    if(state){
        QMessageBox::information(this, tr("ARink"),
                                 tr("File Saved %1.").arg(oFilename));
    } else {
        QMessageBox::warning(this, tr("ARink"),
                                 tr("Cannot save %1.").arg(oFilename));
    }
}

void MainWindow::toGrayscale()
{
    if(img.empty())
        return;
    if(img.channels() == 3) {
        cv::cvtColor(img,img,cv::COLOR_BGR2GRAY);
        imageLabel->setPixmap(QPixmap::fromImage( cvMatToQImage(img)));
    }
}

void MainWindow::toColorize()
{
    if(img.empty())
        return;

    if(img.channels() == 1){
        cv::applyColorMap(img, colorImg, cv::COLORMAP_RAINBOW);
        imageLabel->setPixmap(QPixmap::fromImage(cvMatToQImage(colorImg)));
        img = colorImg;
    } else {
        imageLabel->setPixmap(QPixmap::fromImage(cvMatToQImage(img)));
    }
}


void MainWindow::setImageL(const QImage &dst)
{
//    QImage dest = cvMatToQImage(dst);
    img = QImageToCvMat(dst);
    imageLabel->setPixmap(QPixmap::fromImage(dst));

}


void MainWindow::kelvinEffect()
{
    if(img.empty())
        return;

    if(img.channels() == 1){
        qWarning()<<"No change as image is 1 channel";
    } else {

        KelvinFilter *workerThread = new KelvinFilter(this);
        connect(workerThread, &KelvinFilter::resultReady, this, &MainWindow::setImageL);
        connect(workerThread, &KelvinFilter::finished, workerThread, &QObject::deleteLater);
        workerThread->setInput( img);
        workerThread->start();
//        QThread *thread = new QThread();
//        KelvinFilter *kf = new KelvinFilter();
//        kf->moveToThread( thread );

//        connect( thread, SIGNAL(finished()), kf, SLOT(deleteLater()) );
////        QObject::connect( kf, SIGNAL(result(QImage)), thread, SLOT(deleteLater()) );
//        connect( thread, SIGNAL(started()), kf, SLOT(doWork()) );
//        connect( kf, SIGNAL(result(QImage)), this, SLOT(setImageL(QImage)) );
//        kf->setInput( img);
//        thread->start();
    }
}

void MainWindow::coolEffect()
{
    if(img.empty())
        return;

    if(img.channels() == 1){
        qWarning()<<"No change as image is 1 channel";
    } else {

        CoolFilter *workerThread = new CoolFilter(this);
        connect(workerThread, &CoolFilter::resultReady, this, &MainWindow::setImageL);
        connect(workerThread, &CoolFilter::finished, workerThread, &QObject::deleteLater);
        workerThread->setInput( img);
        workerThread->start();
    }
}




void MainWindow::mosaicEffect()
{
    if(img.empty())
        return;

    if(img.channels() == 1){
        qWarning()<<"No change as image is 1 channel";
    } else {

        MosaicFilter *workerThread = new MosaicFilter(this);
        connect(workerThread, &MosaicFilter::resultReady, this, &MainWindow::setImageL);
        connect(workerThread, &MosaicFilter::finished, workerThread, &QObject::deleteLater);
        workerThread->setGridSize(this->spinBox->value());
        qDebug()<<"Size value "<<this->spinBox->value();
        workerThread->setInput( img);
        workerThread->start();
    }
}


void MainWindow::resetImageOriginal()
{
    if(img.empty())
        return;

    img = originalCopy.clone();
    if(originalCopy.channels() == 3){
        colorImg = originalCopy.clone();
    }
    imageLabel->setPixmap(QPixmap::fromImage(cvMatToQImage(img)));
}




void MainWindow::on_vignetteAct_triggered()
{
    qDebug()<<"Act triggered";


//    qDebug()<<"Pos"<<

//    QWidget* wdslide = new QWidget(ui->mainToolBar);
//    wdslide->setGeometry(50,100,50,100);

//    wdslide->setVisible(true);




//    QSlider* vslide = new QSlider(ui->mainToolBar);
//    vslide->setMinimum(0);
//    vslide->setMaximum(100);
////    vslide->setGeometry(ui->mainToolBar->width()/2,30,30,200);
////    qDebug()<<ui->mainToolBar->mouseGrabber()->x();
////    vslide->setGeometry(ui->mainToolBar->mouseGrabber()->x());


//    vslide->setVisible(true);

}
