//
// Created by rege on 08.03.17.
//

#include <plog/Log.h>
#include <opencv2/opencv.hpp>

#include "TheCarCV.hpp"
#include "Config.hpp"
#include "Detect.hpp"


using namespace std;
using namespace experimental::filesystem;
using namespace cv;

/**
 * warning: before every line, which works with windows, macro “IFWIN” must be put
 * (this lines will be skipped when windows mode will be off)
 * */

TheCarCV::TheCarCV ()
{
    init();
}

bool TheCarCV::isShowingWindows ()
{
    return this->createWindows;
}

void TheCarCV::turnOnWindows ()
{
    // (“IFWIN” macro doesn't need for this function) 

    this->createWindows = true;

    /// creating windows

    for (string & winName : *windows)
    {
        namedWindow(winName, CV_WINDOW_AUTOSIZE);
    }

    /// "control" window set
    // red
    createTrackbar("red LowH", "color filter", &redILowH, 179); //Hue (0 - 179)
    createTrackbar("red HighH", "color filter", &redIHighH, 179);

    createTrackbar("red LowS", "color filter", &redILowS, 255); //Saturation (0 - 255)
    createTrackbar("red HighS", "color filter", &redIHighS, 255);

    createTrackbar("red LowV", "color filter", &redILowV, 255); //Value (0 - 255)
    createTrackbar("red HighV", "color filter", &redIHighV, 255);
    // blue
    createTrackbar("blue LowH", "color filter", &blueILowH, 179); //Hue (0 - 179)
    createTrackbar("blue HighH", "color filter", &blueIHighH, 179);

    createTrackbar("blue LowS", "color filter", &blueILowS, 255); //Saturation (0 - 255)
    createTrackbar("blue HighS", "color filter", &blueIHighS, 255);

    createTrackbar("blue LowV", "color filter", &blueILowV, 255); //Value (0 - 255)
    createTrackbar("blue HighV", "color filter", &blueIHighV, 255);

    int __fakeDp;
    createTrackbar("dp (x0.1)", "color filter", &__fakeDp, 50, [] (int n, void * data)
    {
        double * dp = (double *) data;
        *dp = ((double) n) / 10;
        if (*dp == 0) *dp = 0.1;
    }, (void *) &hDp);

    createTrackbar("minDist", "color filter", &hMinDist, 200);
    createTrackbar("param1", "color filter", &hParam1, 1000);
    createTrackbar("param2", "color filter", &hParam2, 1000);
    createTrackbar("minRadius", "color filter", &hMinRadius, 200);
    createTrackbar("maxRadius", "color filter", &hMaxRadius, 200);

    createTrackbar("threshold", "edges", &edgeThreshold, 100);
}

void TheCarCV::init ()
{
    LOGD << "TheCarCV init…";

    /// video output init

    // names for windows, which will be created
    windows = new vector<string>{"original", "color filter"};

    /// video input init

    LOGD << "try to open a webcam";
    int videoInputSourceNumber = getInt("video/video_input_source_number");
    try
    {
        cap = new VideoCapture(videoInputSourceNumber); //capture the video from web cam
    }
    catch (exception e)
    {
        LOGF << "cannot open webcam " + to_string(videoInputSourceNumber);
        throw (e);
    }
    LOGI << "video input source " + to_string(videoInputSourceNumber) + " has set";

    if (!cap->isOpened())  // if not success, exit program with error
    {
        LOGE << "Cannot open the web cam";
        __throw_system_error(2);
    } else LOGI << "video input source " + to_string(videoInputSourceNumber) + " has opened seccesfully";

    /// configuration set

    redILowH = getInt("hsv_filtering/red/low_h");
    redIHighH = getInt("hsv_filtering/red/high_h");
    redILowS = getInt("hsv_filtering/red/low_s");
    redIHighS = getInt("hsv_filtering/red/high_s");
    redILowV = getInt("hsv_filtering/red/low_v");
    redIHighV = getInt("hsv_filtering/red/high_v");

    blueILowH = getInt("hsv_filtering/blue/low_h");
    blueIHighH = getInt("hsv_filtering/blue/high_h");
    blueILowS = getInt("hsv_filtering/blue/low_s");
    blueIHighS = getInt("hsv_filtering/blue/high_s");
    blueILowV = getInt("hsv_filtering/blue/low_v");
    blueIHighV = getInt("hsv_filtering/blue/high_v");

    hDp = getDouble("hough_circle_transform_parametrs/dp");
    hMinDist = getInt("hough_circle_transform_parametrs/min_dist");
    hParam1 = getInt("hough_circle_transform_parametrs/p1");
    hParam2 = getInt("hough_circle_transform_parametrs/p2");
    hMinRadius = getInt("hough_circle_transform_parametrs/min_radius");
    hMaxRadius = getInt("hough_circle_transform_parametrs/max_radius");


    Detect::getInstance().init();
}

void TheCarCV::start ()
{
    vector<RoadSignData> roadSigns;

    for (;;)
    {
        Mat frame;
        if (!cap->read(frame))
        {
            //LOGE << "Cannot open the web cam";
            __throw_system_error(2);
        }
        if (waitKey(10) == 27)
        {
            break;
        }
        processFrame(frame);
    }

    LOGI << "TheCarCV terminated";
}

void TheCarCV::hsvFilter (
        Mat & imgHSV, Mat & dsc,
        int lh, int hh,
        int ls, int hs,
        int lv, int hv
)
{
    inRange(imgHSV, Scalar(lh, ls, lv), Scalar(hh, hs, hv), dsc); //Threshold the image
}

void TheCarCV::openCVHughCircleTransform (Mat & gray, vector<Vec3f> & circles)
{
    assert(gray.channels() == 1);
    Mat smoothed;
    // smooth it, otherwise a lot of false circles may be detected
    GaussianBlur(gray, smoothed, Size(9, 9), 2, 2);
    HoughCircles(smoothed, circles, HOUGH_GRADIENT,
                 hDp, hMinDist, hParam1, hParam2, hMinRadius, hMaxRadius);
}

// edges detection with controlable trashhold
void TheCarCV::edgeDetect (Mat & src, Mat & dsc)
{
    Mat srcGray;
    cvtColor(src, srcGray, CV_BGR2GRAY);
    int ratio = 7;
    int kernel_size = 3;
    Mat edges;
    Canny(srcGray, edges, edgeThreshold, edgeThreshold * ratio, kernel_size);
    dsc = edges;
}

// if success then returns true else returns false
bool TheCarCV::cutSquareRegionByCircle (Mat & src, Mat & dsc, int x, int y, int radius)
{
    assert(radius > 0);
    int x0 = x - radius;
    int y0 = y - radius;

    // doesn't circle go out of image
    if (x0 < 0 || y0 < 0)
        return false;
    if (x + radius > src.cols || y + radius > src.rows)
        return false;

    dsc = Mat(src, Rect_<int>(x0, y0, 2 * radius, 2 * radius));
    return true;
}


double TheCarCV::getNonzerosToZerosRatio (const cv::Mat & monochromeImg)
{
    assert(monochromeImg.type() == CV_8UC1);
    int countOfZero = 0, countOfNonZero = 0;
    for (uint8_t pixel : Mat_<uint8_t>(monochromeImg))
    {
        if (pixel) countOfNonZero++;
        else countOfZero++;
    }
    return (double) countOfNonZero / (double) countOfZero;
}

void TheCarCV::processFrame (Mat frame)
{
    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);

    Mat imgHSV;

    // takes ~0.006 sec.
    cvtColor(frame, imgHSV, COLOR_BGR2HSV);

    Mat red, blue;

    // takes ~0.0016 sec.
    hsvFilter(imgHSV, red, redILowH, redIHighH, redILowS, redIHighS, redILowV, redIHighV);
    hsvFilter(imgHSV, blue, blueILowH, blueIHighH, blueILowS, blueIHighS, blueILowV, blueIHighV);

    {
        Mat redAndBlueOnly(red.size(), CV_8UC3);
        int red_from_to[] = {0, 2};
        mixChannels(red, redAndBlueOnly, red_from_to, 1);
        int blue_from_to[] = {0, 0, -1, 1};
        mixChannels(blue, redAndBlueOnly, blue_from_to, 2);

        imshow("red and blue only", redAndBlueOnly);
    }

    vector<Vec3f> circles;
    openCVHughCircleTransform(gray, circles);

    for (const Vec3f & c : circles)
    {
        const int minRadius = getInt("hough_circle_transform_parametrs/min_radius");
        if (c[2] < minRadius) continue; // it's double verify for minRadius limit

        Mat redResult, blueResult;

        if (!cutSquareRegionByCircle(red, redResult, c[0], c[1], c[2])) continue;
        if (!cutSquareRegionByCircle(blue, blueResult, c[0], c[1], c[2])) continue;

        double nonzerosToZerosRatioOfRed = getNonzerosToZerosRatio(redResult);
        double nonzerosToZerosRatioOfBlue = getNonzerosToZerosRatio(blueResult);

        static double minRatio = getDouble("detection/ratio_nonzeros_to_zeros");
        // if in both images too small non zero pixels then there is no good shot, is isn't red or blue sign
        if (max(nonzerosToZerosRatioOfRed, nonzerosToZerosRatioOfBlue) < minRatio) continue;

        if (nonzerosToZerosRatioOfBlue > nonzerosToZerosRatioOfRed) // then it's blue sign
        {
            Detect::getInstance().detect(blueResult, RoadSign::Color::BLUE);
            // then draw to main window what we found
            circle(frame, Point(cvRound(c[0]), cvRound(c[1])), cvRound(c[2]), Scalar(200, 50, 30), 3); // ~blue color
        } else
        { // red
            Detect::getInstance().detect(redResult, RoadSign::Color::RED);
            // then draw to main window what we found
            circle(frame, Point(cvRound(c[0]), cvRound(c[1])), cvRound(c[2]), Scalar(60, 20, 230), 3); // ~red color
        }
    }

    IFWIN imshow("original", frame);
}