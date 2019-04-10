//
// Created by user on 15.03.17.
//

#include <opencv2/highgui.hpp>
#include <plog/Log.h>
#include <opencv2/imgproc.hpp>
#include <map>
#include <experimental/filesystem>

#include "Detect.hpp"
#include "Config.hpp"
#include "CNN.hpp"

using namespace std;
using namespace cv;
namespace fs = experimental::filesystem;

void Detect::init ()
{
    baseDirectory = getString("ml/base_directory");
    string originalsSubdirectory = getString("ml/originals_subdirectory");
    originalsDirectory = baseDirectory + "/" + originalsSubdirectory;

    if (baseDirectory.empty())
    {
        LOGF << "There is no configuration/ml/base_directory set in config.xml or I cannot read it.";
        exit(25);
    }
    if (originalsSubdirectory.empty())
    {
        LOGF << "There is no configuration/ml/originals_subdirectory set in config.xml or I cannot read it.";
        exit(26);
    }
}

void Detect::loadData ()
{
    models = Detect::getInstance().loadModels();
}

bool Detect::prepareAnImage (const Mat1b & src, Mat1b & dst)
{
    if (src.rows < IMAGE_WIDTH || src.cols < IMAGE_HEIGHT)
        return false;

    Mat image;

    // resizing

    Mat resized;
    resize(src, resized, Size(IMAGE_HEIGHT, IMAGE_WIDTH));

    // making a negative, and making value range simple to 0-1

    for (uchar & p : Mat1b(resized))
    {
        p = p ? (uchar) 0 : (uchar) 255;
    }

    // applying a circle mask

    Mat1b mask = Mat::zeros(Size(IMAGE_HEIGHT, IMAGE_WIDTH), CV_8UC1);
    circle(mask,
           Point(IMAGE_HEIGHT / 2, IMAGE_WIDTH / 2),
           (int) ((float) IMAGE_HEIGHT / 2.1f),
           Scalar(255),
           -1);
    resized.copyTo(image, mask);

    // saving

    dst = image;

    return true;
}

// takes monochrome image,
// returns a number of road sign (enum of struct RoadSign)
int Detect::detect (Mat1b mat)
{
    CNN cnn;
    Mat1f output;
    cnn.process(mat, output);

    if (output.rows == 0 || output.cols == 0)
    {
        LOGE << "CNN didn't give me its output, therefore I cannot continue detection, therefore I return -1 as error";
        return -1;
    }

    map<int, double> resultOfComparing;
    for (auto const & p : models)
    {
        resultOfComparing[p.first] = compare(output, p.second);
    }
    static double threshold = getDouble("ml/threshold");
    double minimum = threshold; // minimum difference
    int roadSignIsHavingMinimum = RoadSign::NONE;
    string summary = "Detection summary:"; //for logging
    for (auto const & p : resultOfComparing)
    {
        if (p.second < minimum)
        {
            roadSignIsHavingMinimum = p.first;
            minimum = p.second;
        }
        summary = summary + " " + RoadSign::getName(p.first) + "=" + to_string((int) round(p.second));
    }
    LOGD << summary;

    return roadSignIsHavingMinimum;
}

map<int, Mat1f> Detect::makeModels (const map<int, vector<Mat1f>> & cnnOutputBySigns)
{

    // contains:
    // key: road sign number
    // value:
    // ---> key: count of matrix are summarized
    // ---> value: matrix is result of sum
    map<int, pair<float, Mat1f>> temporary;

    for (auto const & p: cnnOutputBySigns)
    {
        if (p.second.empty())
        {
            LOGF << "Somebody gave me empty vector of road sign " + RoadSign::getName(p.first);
            exit(23);
        }

        bool firstRun = true;

        for (auto const & m : p.second)
        {
            if (firstRun)
            {
                temporary[p.first] = make_pair(0.0f, m);
                firstRun = false;
                continue;
            }

            temporary.at(p.first).second += m;
            temporary.at(p.first).first += 1.0f;

            //LOGD << "Sum: " + to_string(sum(temporary.at(p.first).second)[0]);
        }

        temporary.at(p.first).second = temporary.at(p.first).second / temporary.at(p.first).first;
        //LOGD << "Final sum: " + to_string(sum(temporary.at(p.first).second)[0]);
    }

    map<int, Mat1f> result;
    for (auto const & p: temporary)
    {
        result[p.first] = p.second.second;
    }
    return result;
}

void Detect::saveModels (const map<int, Mat1f> & models)
{
    string modelsFilePath = getString("ml/base_directory") + "/models.xml";
    FileStorage fs(modelsFilePath, FileStorage::WRITE);

    // check for models
    for (int s : RoadSign::getRoadSignNumbers())
    {
        try
        {
            if (models.at(s).empty())
            {
                LOGF << "Model for " + RoadSign::getName(s) + " is empty";
                exit(25);
            }
        } catch (out_of_range e)
        {
            LOGF << "No model for road sign " + RoadSign::getName(s);
            exit(26);
        }
    }

    // check for access for file writing
    if (!fs.isOpened())
    {
        LOGF << "Cannot create a file storage (" + modelsFilePath + ")";
        exit(24);
    }

    // write the models to file and close the storage
    for (auto const & p : models)
    {
        fs.write(RoadSign::getName(p.first), p.second);
    }
    fs.release();
}

map<int, Mat1f> Detect::loadModels ()
{
    map<int, Mat1f> result;

    string modelsFilePath = getString("ml/base_directory") + "/models.xml";
    FileStorage fs(modelsFilePath, FileStorage::READ);

    // check for access for file reading
    if (!fs.isOpened())
    {
        LOGF << "Cannot read a file storage (" + modelsFilePath + ")";
        exit(25);
    }

    for (const string & s : RoadSign::getNames())
    {
        fs[s] >> result[RoadSign::getRoadSignNumber(s)];
        if (result.at(RoadSign::getRoadSignNumber(s)).empty())
        {
            LOGF << "A model for road sign " + s + " is empty (" + modelsFilePath + ")";
            exit(26);
        }
    }
    return result;
};

Detect::Detect ()
{}

double Detect::compare (const cv::Mat1f & firstMat, const cv::Mat1f & secondMat)
{
    assert(firstMat.rows != 0 && firstMat.cols != 0 && secondMat.rows != 0 && secondMat.cols != 0);
    assert(firstMat.rows == secondMat.rows && firstMat.cols == secondMat.cols);

    double result = 0.0;

    for (int y = 0; y < firstMat.rows; y++)
    {
        for (int x = 0; x < firstMat.cols; x++)
        {
            result += pow(firstMat.at<float>(y, x) - secondMat.at<float>(y, x), 2.0f) / 2.0f;
        }
    }
    return result;
}

bool Detect::train ()
{
    if (!isOriginalsMinimumFilfulled())
    {
        LOGD << "I cannot train the model because I haven't found all the originals need to me";
        return false;
    }

    CNN cnn;

    map<int, vector<Mat1f>> cnnOutputBySigns;

    for (auto roadSignNumber : RoadSign::getRoadSignNumbers())
    {
        cnnOutputBySigns[roadSignNumber] = vector<Mat1f>{};
        for (auto & signShotFilePath : fs::directory_iterator(
                fs::path(
                        originalsDirectory
                        + "/"
                        + RoadSign::getName(roadSignNumber)))
                )
        {
            // load the image
            Mat1b shot = imread(signShotFilePath.path().string(), IMREAD_GRAYSCALE);
            Mat1f cnnOutput;
            cnn.process(shot, cnnOutput);
            cnnOutputBySigns.at(roadSignNumber).emplace_back(cnnOutput);
        }
    }

    map<int, Mat1f> models = makeModels(cnnOutputBySigns);
    saveModels(models);

    return true;
}

map<int, int> Detect::getStatisticsAboutOriginals ()
{
    map<int, int> stat;

    for (auto roadSignNumber : RoadSign::getRoadSignNumbers())
    {
        stat[roadSignNumber] = 0;
        try
        {
            for (auto & signShotFilePath : fs::directory_iterator(
                    fs::path(
                            originalsDirectory
                            + "/"
                            + RoadSign::getName(roadSignNumber)))
                    )
            {
                // load the image
                stat.at(roadSignNumber)++;
            }
        } catch (fs::filesystem_error e)
        {
            LOGE << e.what();
            LOGE << "Cannot calculate count of originals for road sign " + RoadSign::getName(roadSignNumber)
                    + ". Does directory " + originalsDirectory + "/" + RoadSign::getName(roadSignNumber) + " exist?";
        }
    }

    return stat;
}

bool Detect::isOriginalsMinimumFilfulled ()
{
    auto stat = getStatisticsAboutOriginals();
    for (auto & p : stat)
    {
        if (p.second == 0)
        {
            return false;
        }
    }
    return true;
}

void Detect::createDirectories ()
{
    for (const string & s : RoadSign::getNames())
    {
        try
        {
            fs::path path = originalsDirectory + "/" + s;
            if (!fs::exists(path))
                fs::create_directories(path);
        } catch (fs::filesystem_error e)
        {
            LOGE << e.what();
        }
    }
}