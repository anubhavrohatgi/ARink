#include "filters.h"


KelvinFilter::KelvinFilter(QObject *parent) : QThread(parent)
{
    qDebug()<<"Constructing Kelvin Filter Worker Thread Object";
}

KelvinFilter::~KelvinFilter()
{
    base.release();
    qDebug()<<"Deleting KF";
}


void KelvinFilter::run()
{
    if(base.empty()){
        emit resultReady(QImage());
    }

    if(base.channels() !=3){
        emit resultReady(QImage());
    }


    //Enhance the channel for any image BGR or HSV etc
    cv::Mat img = base.clone();

    // Specifying the x-axis for mapping
    float originalR[] = {0, 60, 110, 150, 235, 255};
    float originalG[] = {0, 68, 105, 190, 255};
    float originalB[] = {0, 88, 145, 185, 255};

    // Specifying the y-axis for mapping
    float rCurve[] = {0, 102, 185, 220, 245, 245};
    float gCurve[] = {0, 68, 120, 220, 255};
    float bCurve[] = {0, 12, 140, 212, 255};
    // Splitting the channels
    std::vector<cv::Mat> channels(3);
    cv::split(img, channels);

    // Create a LookUp Table
    float fullRange[256];
    int i;

    for(i=0;i<256;i++)
    {
        fullRange[i]= (float)i;
    }

    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* lut = lookUpTable.ptr();

    // Apply interpolation and create look up table
    interpolation(lut,fullRange,bCurve,originalB);

    // Apply mapping and check for underflow/overflow in Red Channel
    cv::LUT(channels[0],lookUpTable,channels[0]);

    // Apply interpolation and create look up table
    interpolation(lut,fullRange,gCurve,originalG);

    // Apply mapping and check for underflow/overflow in Blue Channel
    cv::LUT(channels[1],lookUpTable,channels[1]);

    // Apply interpolation and create look up table
    interpolation(lut,fullRange,rCurve,originalR);

    // Apply mapping and check for underflow/overflow in Blue Channel
    cv::LUT(channels[2],lookUpTable,channels[2]);

    cv::Mat output;
    // Merge the channels
    cv::merge(channels,output);

    //Signal the main thread that the results are computed
    emit resultReady(cvMatToQImage(output) );
}


void KelvinFilter::setInput(const cv::Mat &input)
{
    base = input.clone();
}


//=========================== COOL FILTER =====================


CoolFilter::CoolFilter(QObject *parent) : QThread(parent)
{
    qDebug()<<"Constructing Cool Filter Worker Thread Object";
}

CoolFilter::~CoolFilter()
{
    Image.release();
    qDebug()<<"Deleting Cool Filter";
}


void CoolFilter::setInput(const cv::Mat &input)
{
    Image = input.clone();
}


void CoolFilter::run()
{
    if(Image.empty()){
        emit resultReady(QImage());
    }

    if(Image.channels() !=3){
        emit resultReady(QImage());
    }

    // Pivot points for X-Coordinates
    float originalValue[] = {0,50,100,150,200,255};

    // Changed points on Y-axis for each channel
    float bCurve[] = {0,80,150,190,220,255};
    float rCurve[] = {0,20,40,75,150,255};

    // Splitting the channels
    std::vector<cv::Mat> channels(3);
    cv::split(Image, channels);

    // Create a LookUp Table
    float fullRange[256];
    int i;
    for(i=0;i<256;i++)
    {
        fullRange[i]= (float)i;
    }
    cv::Mat lookUpTable(1, 256, CV_8U);
    uchar* lut = lookUpTable.ptr();

    // Apply interpolation and create look up table
    interpolation(lut,fullRange,rCurve,originalValue);

    // Apply mapping and check for underflow/overflow in Red Channel
    cv::LUT(channels[2],lookUpTable,channels[2]);
    cv::min(channels[2],255,channels[2]);
    cv::max(channels[2],0,channels[2]);

    // Apply interpolation and create look up table
    interpolation(lut,fullRange,bCurve,originalValue);

    // Apply mapping and check for underflow/overflow in Blue Channel
    cv::LUT(channels[0],lookUpTable,channels[0]);
    cv::min(channels[0],255,channels[0]);
    cv::max(channels[0],0,channels[0]);

    cv::Mat output;
    // Merge the channels
    cv::merge(channels,output);

    //Signal the main thread that the results are computed
    emit resultReady(cvMatToQImage(output));

}


