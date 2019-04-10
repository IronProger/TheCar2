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

enum Mode
{
    VIDEO, DETECTION, COLLECT, LEARNING
};
/* About mode from config.xml
 * <!-- Sets what the application needs to do
It can be:
 - video - shows only a video stream and does nothing
 - quiet_detection - the detection mode without GUI
 - detection - the detection mode with GUI
 - collect - the mode for collecting data for learning (shows a video stream)
 - learning - learns on the collected data -->
 */

// singleton
class TheCarCV
{
private:
    Mode mode = VIDEO;

    int blueILowH, blueIHighH, blueILowS, blueIHighS, blueILowV, blueIHighV;

    double hDp;
    int hMinDist, hParam1, hParam2, hMinRadius, hMaxRadius;

    int edgeThreshold;

    // will show GUI
    // warning: don't forget insert "if (show)" before each line is related with GUI
    bool show;

    // don't forget to insert "if (show && showExtra)" before every line working with windows except the main window
    bool showExtra;

    cv::VideoCapture * cap;

    TheCarCV ();

    void hsvFilter (
            cv::Mat & src, cv::Mat & dsc,
            int lh, int hh,
            int ls, int hs,
            int lv, int hv
    );

    bool cutSquareRegionByCircle (cv::Mat & src, cv::Mat & dsc, int x, int y, int radius);

    void openCVHughCircleTransform (cv::Mat & src, vector<cv::Vec3f> & circles);

    /**
     *
     * @param frame - the shot
     * @return true if needs to close the application, else false
     */
    bool processFrame (const cv::Mat3b & frame);

    double getNonzerosToZerosRatio (const cv::Mat & monochromeImg);

    /**
     * Gets an image, expands its value diapason from 0-1 to 0-255 and saves it as .png
     * @param preparedImage - a prepared image (monochrome, CV_U8C1, with pixels are only =0 or =1)
     */
    void saveAnImage (const cv::Mat1b & preparedImage);

    // initialize windows and turn on showing of it
    void setUpWindows ();

public:

    static TheCarCV & getInstance ()
    {
        static TheCarCV instance;
        return instance;
    }

    /**
     * Must be called before start from main.cpp
     */
    void init ();

    /**
     * Main start point
     */
    void start ();

    /**
     * It was helpful to show red and blue together in one window when I used to pass red and blue colors standalone
     * @param red - one channel image (red is implied)
     * @param blue - one channel image (blue is implied)
     * @param dst - three channel image containing red, blue and zero green
     */
    void uniteRedAndBlue (const cv::Mat1b & red, const cv::Mat1b & blue, cv::Mat3b & dst);
};

#endif //THECAR_THECARCV_HPP
