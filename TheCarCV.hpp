//
// Created by rege on 08.03.17.
//

#ifndef THECAR_THECARCV_HPP
#define THECAR_THECARCV_HPP

#include <string>
#include <vector>
#include <experimental/filesystem>
#include <opencv2/opencv.hpp>

using namespace std;

#define IFWIN if (TheCarCV::getInstance().isShowingWindows())

const int RESOLUTION_OF_IMAGE_FOR_DETECTION = 50;

typedef struct RoadSignData
{
    int mat[RESOLUTION_OF_IMAGE_FOR_DETECTION][RESOLUTION_OF_IMAGE_FOR_DETECTION];
    int distance;
} RoadSignData;

// singleton
class TheCarCV
{
private:
    int redILowH, redIHighH, redILowS, redIHighS, redILowV, redIHighV;

    int blueILowH, blueIHighH, blueILowS, blueIHighS, blueILowV, blueIHighV;

    // hough circle transform parameters
    double hDp;
    int hMinDist, hParam1, hParam2, hMinRadius, hMaxRadius;

    int edgeThreshold;

    bool createWindows;

    cv::VideoCapture * cap;
    vector<string> * windows;

    TheCarCV ();

    TheCarCV (const TheCarCV &);

    TheCarCV & operator= (TheCarCV &);

    void init ();

    void hsvFilter (
            cv::Mat & src, cv::Mat & dsc,
            int lh, int hh,
            int ls, int hs,
            int lv, int hv
    );

    void edgeDetect (cv::Mat & src, cv::Mat & dsc);

    bool cutSquareRegionByCircle (cv::Mat & src, cv::Mat & dsc, int x, int y, int radius);

    inline bool cutSquareRegionByCircle (cv::Mat & src, cv::Mat & dsc, cv::Vec3f circle);

    void openCVHughCircleTransform (cv::Mat & src, vector<cv::Vec3f> & circles);

    void processFrame (cv::Mat frame);

public:

    static TheCarCV & getInstance ()
    {
        static TheCarCV instance;
        return instance;
    }

    // initialize windows and turn on showing of it
    void turnOnWindows ();

    // if windows are showing on screen return true, else false
    bool isShowingWindows ();

    // warning: this method block the thread (road sign detection start)
    // thread must be unlocked only at program shutdown
    void start ();
};

#endif //THECAR_THECARCV_HPP
