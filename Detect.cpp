//
// Created by user on 15.03.17.
//

#include <opencv2/highgui.hpp>
#include <plog/Log.h>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <experimental/filesystem>

#include "Detect.hpp"
#include "Config.hpp"

using namespace std;
using namespace cv;
using namespace experimental::filesystem;

void Detect::init ()
{
    string samplesDirectory = getString("files/road_sign_samples_directory");

    auto addToMap = [=] (int sign, string confPath)
    {
        vector<Mat> images;
        for (string & filename : getStringVector(confPath))
        {
            Mat mat = imread(samplesDirectory + filename, IMREAD_GRAYSCALE);
            if (mat.empty())
            {
                LOGE << "file " + samplesDirectory + filename + " doesn't exist";
                continue;
            }
            images.emplace_back(mat);
        }
        samples[sign] = getModelFromImages(images);
    };

    addToMap(RoadSign::RIGHT_TURN_ONLY, "files/road_sign_samples/right_turn_only");
    addToMap(RoadSign::RIGHT_TURN_OR_FORWARD_ONLY, "files/road_sign_samples/right_turn_or_forward_only");
    addToMap(RoadSign::STOP, "files/road_sign_samples/stop");
    addToMap(RoadSign::DO_NOT_ENTER, "files/road_sign_samples/do_not_enter");

    detectionThreshold = getDouble("detection/threshold");
}

// return a number of road sign (enum of struct RoadSign)
int Detect::detect (cv::Mat mat, RoadSign::Color color)
{
    assert(mat.channels() == 1);
    Mat1d image;
    convertToNormalizedModel(mat, image);

    // apply circle mask
    Mat1b mask = Mat::zeros(Size(modelWidth, modelHeight), CV_8UC1);
    circle(mask, Point(modelWidth / 2, modelHeight / 2), modelWidth / 2, Scalar(255), -1);
    Mat1d masked;
    image.copyTo(masked, mask);

    // will images be saved
    static bool save = getBool("files/save_test_images");
    // the directory with will be used to save test images
    static string imDirectory = getString("files/test_output_directory");
    if (save)
    {
        // prepare OS
        // experimental::filesystem is used here
        static bool checked = false;
        if (!checked)
        {
            if (!exists(path(imDirectory)))
            {
                create_directory(path(imDirectory));
            }
            checked = true;
        }
        static int a = 0;
        imwrite(imDirectory + "/" + to_string(++a) + "-masked.png", masked);
    }

    // detect
    double maximum = 0;
    int sign = 0;
    for (const pair<int, Mat1d> & p : samples)
    {
        // color filter (It improve detection quality separating red from blue)
        if (RoadSign::whatIsColor(p.first) != color) continue;

        assert(p.second.type() == masked.type());
        assert(p.second.rows == masked.rows && p.second.cols == masked.cols);
        Mat1d result = p.second.mul(masked);
        double s = 0.0;
        for (double d : result)
        {
            if (!isnan(d) && !isinf(d)) s += d;
        }
        if (s > maximum)
        {
            maximum = s;
            sign = p.first;
        }
    }

    if (maximum > detectionThreshold)
    {
        LOGI << "Detected " + RoadSign::getName(sign) + " (value " + to_string(maximum) + ")";
    }
}

Detect::Detect ()
{}

cv::Mat1d Detect::getModelFromImages (std::vector<Mat> images)
{
    Mat1d result = Mat::zeros(Size(modelWidth, modelHeight), CV_64FC1);
    for (Mat & image : images)
    {
        Mat1d temporary;
        convertToNormalizedModel(image, temporary);
        result = (result + temporary);
    }
    normalize(result, result, 1, 0, cv::NORM_L1);
    return result;
}

void Detect::convertToNormalizedModel (cv::Mat & src, cv::Mat & dst)
{
    assert(src.channels() == 1);
    Mat resized;
    resize(src, resized, Size(modelWidth, modelHeight));
    dst = Mat1d(resized);
}

