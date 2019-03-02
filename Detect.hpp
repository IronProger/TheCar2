//
// Created by rege on 13.03.17.
//

#ifndef THECAR_NN_HPP
#define THECAR_NN_HPP

#include <vector>
#include <map>
#include <opencv2/core/mat.hpp>

// static only
struct RoadSign
{
    enum Color
    {
        RED, BLUE, UNKNOWN
    };

    enum
    {
        FORWARD_ONLY, RIGHT_TURN_ONLY, LEFT_TURN_ONLY, RIGHT_TURN_OR_FORWARD_ONLY, STOP, DO_NOT_ENTER, NONE
    };

    static Color whatIsColor (int roadSign)
    {
        switch (roadSign)
        {
            case FORWARD_ONLY:
            case RIGHT_TURN_ONLY:
            case LEFT_TURN_ONLY:
            case RIGHT_TURN_OR_FORWARD_ONLY:
                return BLUE;
            case STOP:
            case DO_NOT_ENTER:
                return RED;
            default:
                return UNKNOWN;
        }
    }

    static std::string getName (int roadSign)
    {
        switch (roadSign)
        {
            case FORWARD_ONLY:
                return "forward_only";
            case RIGHT_TURN_ONLY:
                return "right_turn_only";
            case LEFT_TURN_ONLY:
                return "left_turn_only";
            case RIGHT_TURN_OR_FORWARD_ONLY:
                return "right_turn_or_forward_only";
            case STOP:
                return "stop";
            case DO_NOT_ENTER:
                return "do_not_enter";
            case NONE:
                return "NONE";
            default:
                return "UNKNOWN";
        }
    }


};

// singleton
class Detect
{
private:
    const int modelHeight = 30;
    const int modelWidth = 30;

    double detectionThreshold;

    std::map<int, cv::Mat1d> samples;

    Detect ();

    Detect (const Detect & d)
    { assert(false); };

    Detect & operator= (const Detect & d)
    { assert(false); };

    // gets right images of one sign,
    // makes a mat with resolution 30x30 with mealing double points of input images
    cv::Mat1d getModelFromImages (std::vector<cv::Mat> images);

    void convertToNormalizedModel (cv::Mat & src, cv::Mat & dst);

public:
    static Detect & getInstance ()
    {
        static Detect d;
        return d;
    }

    void init ();

    int detect (cv::Mat monochromeImage, RoadSign::Color color);
};


#endif //THECAR_NN_HPP
