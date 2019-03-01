#include <iostream>

#include <plog/Log.h>

#include "TheCarCV.hpp"
#include "Config.hpp"

using namespace std;

int main (int arg, char ** argv)
{
    /// init configuration

    Config::getInstance().init("../config.xml");

    /// init log

    plog::init(plog::verbose, getString("files/logfile").c_str());
    LOGD << "started";

    /// conf TheCar and start it

    if (getBool("gui/show_windows"))
    {
        LOGI << "windows showing is enabled";
        TheCarCV::getInstance().turnOnWindows();
    }

    TheCarCV::getInstance().start();

    return 0;
}