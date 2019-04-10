//
// Created by rege on 06.03.19.
//

#ifndef THECAR_STOPSIGNDETECT_HPP
#define THECAR_STOPSIGNDETECT_HPP

#include "opencv2/core.hpp"

// singleton
class StopSignDetect
{
private:
    StopSignDetect ()
    {};

    cv::Vec3i find (std::vector<cv::Vec3i>);

public:
    static StopSignDetect getInstance ()
    {
        static StopSignDetect d;
        return d;
    }

    void init ();
};


#endif //THECAR_STOPSIGNDETECT_HPP
