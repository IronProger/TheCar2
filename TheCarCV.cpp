//
// Created by rege on 08.03.17.
//

#include <plog/Log.h>
#include <opencv2/opencv.hpp>

#include "TheCarCV.hpp"
#include "Config.hpp"
#include "Detect.hpp"
#include "speedtest.h"


using namespace std;
using namespace experimental::filesystem;
using namespace cv;

TheCarCV::TheCarCV ()
{}

void TheCarCV::setUpWindows ()
{
    /// creating windows
    namedWindow("original", CV_WINDOW_AUTOSIZE);

    if (showExtra)
    {
        namedWindow("color filter", CV_WINDOW_AUTOSIZE);

        /// set up "control" window
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
}

void TheCarCV::init ()
{
    LOGD << "TheCarCV init…";

    string mode = getString("mode");
    if (mode == "quiet_detection")
    {
        this->mode = DETECTION;
        this->show = false;
    } else if (mode == "learning")
    {
        this->mode = LEARNING;
        this->show = false;
    } else if (mode == "detection")
    {
        this->mode = DETECTION;
        this->show = true;
    } else if (mode == "collect")
    {
        this->mode = COLLECT;
        this->show = true;
    } else if (mode == "quiet_collect")
    {
        this->mode = COLLECT;
        this->show = false;
    } else if (mode == "video")
    {
        this->mode = VIDEO;
        this->show = true;
    } else
    {
        LOGF << "You gave me mode \"" + mode + "\". Please check it in config.xml because\
                    I don't know what this mode means me to do. I'll just terminate.";
        exit(20);
    }
    LOGI << "I'm working in mode \"" + mode + "\"";

    showExtra = getBool("gui/show_extra_windows");

    switch (this->mode)
    {
        case DETECTION:
            Detect::getInstance().init();
            Detect::getInstance().loadData();
            break;
        case COLLECT:
            Detect::getInstance().init();
            Detect::getInstance().createDirectories();
            break;
        case LEARNING:
            Detect::getInstance().init();
            break;
    }

    if (this->mode != LEARNING)
    {
        // All this initialization doesn't need if the application starts for learning

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

        /// load some data from the config

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


        if (show) setUpWindows();
    }
}

void reinforceBlueUsingRed (const Mat3b & src, Mat3b & dst)
{
    typedef Point3_<unsigned char> Pixel;
    Mat3b result(src.rows, src.cols);
    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            Pixel p = src.at<Pixel>(y, x);
            p.x = max(p.x, p.z);
            result.at<Pixel>(y, x) = p;
        }
    }
    dst = result;
}

void TheCarCV::start ()
{
    if (mode == LEARNING)
    {
        SPEEDTEST_("learning", Detect::getInstance().train());
        LOGI << "Learning ends. I have to terminate the program.";
        return;
    }

    for (;;)
    {
        Mat frame;
        static int cannotFetAFrame = 0;
        if (!cap->read(frame))
        {
            cannotFetAFrame++;
            // if cannotGetAFrame is too big then it's a serious error, let big is 10
            if (cannotFetAFrame == 10)
            {
                LOGF << "Cannot get a frame from the web cam";
                __throw_system_error(2);
            }
        }
        cannotFetAFrame = 0;

        if (waitKey(10) == 27)
        {
            break;
        }
        if (processFrame(frame))
        {
            break;
        }
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

void TheCarCV::saveAnImage (const Mat1b & preparedImage)
{
    assert(preparedImage.rows != 0 || preparedImage.cols != 0);

    Mat1b toSave;
    preparedImage.copyTo(toSave);

    for (uchar & p : Mat1b(toSave))
    {
        p = p ? (uchar) 255 : (uchar) 0;
    }

    // the baseDirectory with will be used to save test images
    static string outputDirectory = getString("files/output_directory");
    // prepare OS
    // experimental::filesystem is used here
    static bool checked = false;
    if (!checked)
    {
        if (!exists(path(outputDirectory)))
        {
            create_directory(path(outputDirectory));
        }
        checked = true;
    }
    static int a = 0;

    string filepath = outputDirectory + "/" + to_string(++a) + ".png";
    LOGI << "saving " + filepath;
    imwrite(filepath, toSave);
}

void TheCarCV::uniteRedAndBlue (const Mat1b & red, const Mat1b & blue, Mat3b & dst)
{
    assert(red.rows != 0 && red.cols != 0 && blue.rows != 0 && blue.cols != 0);
    assert(red.rows == blue.rows && red.cols == blue.cols);

    Mat redAndBlueOnly(red.size(), CV_8UC3);
    int red_from_to[] = {0, 2};
    mixChannels(red, redAndBlueOnly, red_from_to, 1);
    int blue_from_to[] = {0, 0, -1, 1};
    mixChannels(blue, redAndBlueOnly, blue_from_to, 2);

    dst = redAndBlueOnly;
}

bool TheCarCV::processFrame (const Mat3b & frame)
{
    Mat3b reinforcedBlue;
    reinforceBlueUsingRed(frame, reinforcedBlue);

    Mat gray;
    cvtColor(reinforcedBlue, gray, CV_BGR2GRAY);

    Mat imgHSV;

    cvtColor(reinforcedBlue, imgHSV, COLOR_BGR2HSV);

    Mat blue;
    hsvFilter(imgHSV, blue, blueILowH, blueIHighH, blueILowS, blueIHighS, blueILowV, blueIHighV);
    if (show && showExtra) imshow("blue", blue);

    vector<Vec3f> circles;
    openCVHughCircleTransform(gray, circles);

    for (const Vec3f & c : circles)
    {
        const int minRadius = getInt("hough_circle_transform_parametrs/min_radius");
        if (c[2] < minRadius) continue; // it's double verify for minRadius limit

        Mat cut;

        if (!cutSquareRegionByCircle(blue, cut, c[0], c[1], c[2])) continue;

        double rationofNonzerosToZeros = getNonzerosToZerosRatio(cut);

        static double minRatio = getDouble("detection/ratio_nonzeros_to_zeros");
        // if in both images too small non zero pixels then there is no good shot, is isn't red or blue sign
        if (rationofNonzerosToZeros < minRatio) continue;

        int sign = RoadSign::NONE;

        if (mode != VIDEO)
        {
            Mat1b preparedImage;
            Detect::getInstance().prepareAnImage(cut, preparedImage);

            if (mode == COLLECT)
            {
                saveAnImage(preparedImage);
            } else if (mode == DETECTION)
            {
                sign = Detect::getInstance().detect(preparedImage);
                LOGI << "Detected: " + RoadSign::getName(sign);
            }
        }

        // Mark the circle was found if it contain minimal necessary value
        // of blue or red color inside so that to be passed to detection
        circle(
                reinforcedBlue,
                Point(cvRound(c[0]), cvRound(c[1])),
                cvRound(c[2]),
                Scalar(50, 200, 30),  // ~green color
                3
        );


        if (mode == DETECTION)
        {
            putText(reinforcedBlue,
                    "Found: " + RoadSign::getName(sign),
                    Point(5, 20), // coordinates
                    FONT_HERSHEY_COMPLEX_SMALL, // font
                    1.0, // scale
                    Scalar(0, 0, 200),
                    1, // thickness
                    CV_AA
            ); // BGR color
        }
    }

    if (show) imshow("original", reinforcedBlue);

    return false;
}