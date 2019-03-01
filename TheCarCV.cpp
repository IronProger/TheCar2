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

    LOGD << "trying to set a webcam";
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

// if success then return true else return false
// also return false if src is not a trust image for detection (smaller than 2/3 of square if white)
// do not resize a result image — it's already resized to RESOLUTION_OF_IMAGE_FOR_DETECTION
bool TheCarCV::cutSquareRegionByCircle (Mat & src, Mat & dsc, int x, int y, int radius)
{
    // first math rect x and y, height and width
    int rectX, rectY, rectHeight, rectWidth;
    rectX = x - radius;
    rectY = y - radius;
    rectHeight = radius * 2;
    rectWidth = radius * 2;

    // then cat an square
    Rect rect(rectX, rectY, rectWidth, rectHeight);

    Mat cutRect;
    if (0 <= rectX && 0 <= rectWidth && rectX + rectWidth <= src.cols
        && 0 <= rectY && 0 <= rectHeight && rectY + rectHeight <= src.rows)
    {
        src(rect).copyTo(cutRect);
    } else
    {
        //LOGW << "expression (0 <= rectX && 0 <= rectWidth && rectX + rectWidth <= src.cols && 0 <= rectY && 0 <= rectHeight && rectY + rectHeight <= src.rows) returned false";
        return false;
    }

    Mat resultThresholded;
    threshold(cutRect, resultThresholded, 170, 255, CV_THRESH_BINARY);
    if (countNonZero(resultThresholded) <
        (RESOLUTION_OF_IMAGE_FOR_DETECTION * RESOLUTION_OF_IMAGE_FOR_DETECTION * 0.7))
    {
        return false;
    }

    dsc = cutRect;
    return true;
}

inline bool TheCarCV::cutSquareRegionByCircle (Mat & src, Mat & dsc, Vec3f circle)
{
    return cutSquareRegionByCircle(src, dsc, circle[0], circle[1], circle[2]);
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

    Mat redAndBlueOnly;
    bitwise_or(red, blue, redAndBlueOnly);

    imshow("red and blue only", redAndBlueOnly);

    vector<Vec3f> circles;
    openCVHughCircleTransform(gray, circles);

    for (Vec3f & c : circles)
    {
        Mat result;

        if (cutSquareRegionByCircle(redAndBlueOnly, result, (const Vec3f) c))
        {
            circle(frame, Point(cvRound(c[0]), cvRound(c[1])), cvRound(c[2]), Scalar(50, 150, 0), 3); // ~green color

            int countOfZero = 0, countOfNonZero = 0;
            for (uint8_t pixel : Mat_<uint8_t>(result))
            {
                if (pixel) countOfNonZero++;
                else countOfZero++;
            }
            double currentRatio = (double) countOfNonZero / (double) countOfZero;
            double minRatio = getDouble("detection/ratio_nonzeros_to_zeros");
            //LOGI << "countOfNonZero/countOfZero="+to_string((double) countOfNonZero/ (double) countOfZero);

            // if in image too small non zero pixels then it isn't correct shot, is isn't red or blue sign
            if (currentRatio > minRatio)
            {
                Detect::getInstance().detect(result);
            }
        }

    }

    IFWIN imshow("original", frame);
}
