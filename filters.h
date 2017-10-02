#ifndef FILTERS_H
#define FILTERS_H


#include <QDebug>
#include <QThread>
#include <QObject>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "interpolation.h"
#include "ocvhelpers.h"


/**
 * @brief The KelvinFilter class
 *  Kelving filter worker thread
 *  Applies Warming effect on the image
 */
class KelvinFilter : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief KelvinFilter KF Constructor
     * @param parent Parent object if any
     */
    KelvinFilter( QObject *parent=0 );

    //Destructor
    ~KelvinFilter();

    /**
     * @brief setInput Set the input image
     * @param input Input 3 channel image
     */
    void setInput( const cv::Mat& input );

signals:
    /**
     * @brief resultReady Signals the main thread once the output is ready
     * @param dst Output 3 channel image
     */
    void resultReady( const QImage& dst );


private:
    cv::Mat base;


protected:
    /**
     * @brief run Overriding the run function of the thread class
     */
    void run() override;

};


/**
 * @brief The CoolFilter class
 *  Cool filter worker thread
 *  Applies Cooling effect on the image
 */
class CoolFilter : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief CoolFilter CF constructor
     * @param parent Parent Object if any
     */
    CoolFilter( QObject *parent=0 );

    //Destructor
    ~CoolFilter();

    /**
     * @brief setInput Set the input image
     * @param input Input 3 channel image
     */
    void setInput( const cv::Mat& input );

signals:

    /**
     * @brief resultReady Signals the main thread once the output is ready
     * @param dst Output 3 channel image
     */
    void resultReady(const QImage& dst);

private:
    cv::Mat Image;

protected:

    /**
     * @brief run Overriding the run function of the thread class
     */
    void run() override;
};








#endif // FILTERS_H
