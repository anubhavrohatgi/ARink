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


//======================== Mosaic Filter ==================================


class ParallelMosaicBody : public cv::ParallelLoopBody
{
    cv::Mat *                      _img;
    const std::vector<cv::Vec6f> * _triangleList;
public:
    ParallelMosaicBody(cv::Mat& img, const std::vector<cv::Vec6f>& triangleList)
        : _img(&img)
        , _triangleList(&triangleList)
    {
    }

    void updateWithAverageColor(cv::Mat& img, const cv::Vec6f& t) const
    {
        std::vector<cv::Point> pt = {
            cv::Point(t[0], t[1]),
            cv::Point(t[2], t[3]),
            cv::Point(t[4], t[5])
        };

        cv::Rect r = cv::boundingRect(pt);
        if (r.x < 0 || r.y < 0
            || (r.x + r.width) > img.cols
            || (r.y + r.height) > img.rows)
            return;

        cv::Mat_<uint8_t> mask(r.height, r.width);

        for (int j = r.x; j < r.x + r.width; j++)
            for (int i = r.y; i < r.y + r.height; i++)
                if (cv::pointPolygonTest(pt, cv::Point(j,i), false) >= 0)
                    mask(i - r.y, j - r.x) = 1;
                else
                    mask(i - r.y, j - r.x) = 0;

        cv::Scalar avg = cv::mean(img(r), mask);
        img(r).setTo(avg, mask);
    }

    void operator()(const cv::Range &r) const override
    {
        auto& img = *_img;
        const auto& triangleList = *_triangleList;

        for (int i = r.start; i < r.end; i++)
        {
            auto& facet = triangleList[i];
            updateWithAverageColor(img, facet);
        }
    }
};


MosaicFilter::MosaicFilter(QObject *parent) : QThread(parent),gridSize(32)
{
    qDebug()<<"Constructing Mosaic Filter Worker Thread Object";
}

MosaicFilter::~MosaicFilter()
{
    mimg.release();
    qDebug()<<"Deleting Mosaic Filter";
}


void MosaicFilter::setInput(const cv::Mat &input)
{
    mimg = input.clone();
}

void MosaicFilter::setGridSize(const int &sz)
{
    gridSize = sz;
}


void MosaicFilter::computeTriangleGrid(
    std::vector<cv::KeyPoint>& grid,
    cv::Rect frame,
    int gridSize,
    int pointOffset)
{
    grid.clear();
    const int w = std::ceil(frame.width / (float)gridSize);
    const int h = std::ceil(frame.height / (float)gridSize);

    cv::RNG rnd;

    // Add four seed points starting at image corners
    grid.push_back(cv::KeyPoint(0, 0, 0));
    grid.push_back(cv::KeyPoint(frame.width-1, 0, 0));
    grid.push_back(cv::KeyPoint(0, frame.height-1, 0));
    grid.push_back(cv::KeyPoint(frame.width-1, frame.height-1, 0));

    for (int i = 0; i <= w; i++)
    {
        for (int j = 0; j <= h; j++)
        {
            int x = i * gridSize + (j % 2 ? gridSize/2 : 0);
            int y = j * gridSize;

            if (pointOffset > 0 && (i > 0 && j > 0 && i < w && j < h))
            {
                x += rnd.uniform(-pointOffset, pointOffset);
                y += rnd.uniform(-pointOffset, pointOffset);
            }

            cv::Point pt(std::max(0,std::min(frame.width-1,x)),
                         std::max(0,std::min(frame.height-1,y)));

            grid.emplace_back(pt, 0);
        }
    }
}


void MosaicFilter::run()
{

    if(mimg.empty()){
        emit resultReady(QImage());
    }

    if(mimg.channels() !=3){
        emit resultReady(QImage());
    }

//    cv::Mat mimg;

    const cv::Rect frame(0,0, mimg.cols, mimg.rows);

    cv::Mat gray;
    cv::cvtColor(mimg, gray, cv::COLOR_BGR2GRAY);
    auto detector = cv::GFTTDetector::create(1024, 0.05, gridSize/2);

    std::vector<cv::KeyPoint> keypoints;
    detector->detect(gray, keypoints);

    // Remove near-border keypoints
    cv::KeyPointsFilter::runByImageBorder(
        keypoints,
        frame.size(),
        gridSize/3);

    // Create regular grid for better mosaicing
    std::vector<cv::KeyPoint> grid;
    computeTriangleGrid(grid, frame, gridSize, gridSize/2);

    // Insert detected keypoints to employ image structure
    for (auto kp: keypoints) {
        // Convert keypoints to integer
        cv::Point pt = kp.pt;
        grid.emplace_back(pt,0);
    }

    // Remove duplicate keypoints to avoid appear
    // of duplicate faces in subdivision
    cv::KeyPointsFilter::removeDuplicated(grid);

    cv::Subdiv2D subdiv(frame);
    for (auto keypoint: grid)
        subdiv.insert(keypoint.pt);

    // Convert image to floating point to reduce preicision loss during averaging
    cv::Mat sourcef;
    mimg.convertTo(sourcef, CV_32F);

    // Fill mosaic
    std::vector<cv::Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);

    cv::parallel_for_(cv::Range(0, triangleList.size()),
                      ParallelMosaicBody(sourcef, triangleList), 32);

    // Convert image to back to byte range
    cv::Mat mosaic;
    sourcef.convertTo(mosaic, CV_8U);

//    qDebug()<<"Here"<<mosaic.channels();


    //Signal the main thread that the results are computed
    emit resultReady(cvMatToQImage(mosaic));
//    return mosaic;
}
