#include <iostream>

#include <plog/Log.h>

#include "TheCarCV.hpp"
#include "Config.hpp"

using namespace std;

int main (int arg, char ** argv)
{
    /// init configuration

    Config::getInstance().init("config.xml");

    /// init log

    string loglevel = getString("files/loglevel");
    plog::Severity maxSeverity = plog::verbose;
    if (loglevel == "none") maxSeverity = plog::none;
    else if (loglevel == "fatal") maxSeverity = plog::fatal;
    else if (loglevel == "error") maxSeverity = plog::error;
    else if (loglevel == "warning") maxSeverity = plog::warning;
    else if (loglevel == "info") maxSeverity = plog::info;
    else if (loglevel == "debug") maxSeverity = plog::debug;
    else if (loglevel == "verbose") maxSeverity = plog::verbose;

    const plog::util::nchar * fileName = getString("files/logfile").c_str();
    plog::init(maxSeverity, fileName);
    LOGD << "start";

    /// init TheCarCV

    TheCarCV::getInstance().init();

    /// start

    TheCarCV::getInstance().start();

    return 0;
}