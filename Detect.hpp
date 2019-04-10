//
// Created by rege on 13.03.17.
//

#ifndef THECAR_NN_HPP
#define THECAR_NN_HPP

#include <vector>
#include <map>
#include <opencv2/core/mat.hpp>
#include "RoadSign.hpp"

// the resolution of prepared image
const int IMAGE_WIDTH = 30;
const int IMAGE_HEIGHT = 30;

// singleton
class Detect
{
private:
    std::map<int, cv::Mat1f> models;

    std::string originalsDirectory;
    std::string baseDirectory;

    Detect ();

    /**
     * Creates one model by list of models using averaging them
     * @param cnnOutputBySigns - cnnOutput, sorted in map by signs
     * @return averaging models im map by signs
     */
    std::map<int, cv::Mat1f> makeModels (const std::map<int, std::vector<cv::Mat1f>> & cnnOutputBySigns);

    /**
     * Loads previously saved models form a file
     * @return loaded models (key: road sign number; value: model)
     */
    std::map<int, cv::Mat1f> loadModels ();

    /**
     * Compares two matrices using standard deviation
     * @returns the difference between matrices
     */
    double compare (const cv::Mat1f & firstMat, const cv::Mat1f & secondMat);

    /**
     * Saves models to a file
     * @param models (key: road sign number; value: model)
     */
    void saveModels (const std::map<int, cv::Mat1f> & models);

public:
    static Detect & getInstance ()
    {
        static Detect d;
        return d;
    }

    void init ();

    /**
     * The last vertion of road signs detection algorithm
     * @param mat - a prepared image by prepareAnImage function
     * @return number of RoadSign
     */
    int detect (cv::Mat1b mat);

    /**
     * Prepares image for detection by: resizing it, making it negative and applying a circle mask to it.
     * @param src - source image (monochrome)
     * @param dst - destination
     * @return true if success
     */
    bool prepareAnImage (const cv::Mat1b & src, cv::Mat1b & dst);

    /**
     * Needs to call one at program start in detection mode because it loads previously saved trained models.
     */
    void loadData ();

    /**
     * Using originals of originals directory divided by road sign names makes averaging models
     * that will be used in road sign detection detection. To make models using this function first
     * collect shots in collection mode and spread it by folders named road sign names (it'll be created
     * automatically with run in collection mode)
     * @return
     */
    bool train ();

    /**
     * Checks is there at least one original image saved for each road sign
     * @returns false if at least one road sign has no original image
     */
    bool isOriginalsMinimumFilfulled ();

    /**
     * Counts number of saved images for every road sign
     * @returns how many images are saved for every road sign
     */
    std::map<int, int> getStatisticsAboutOriginals ();

    /**
     * Creates directories for originals if it is necessary.
     * Please run it in collection mode.
     */
    void createDirectories ();
};


#endif //THECAR_NN_HPP
