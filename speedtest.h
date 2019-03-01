//
// Created by rege on 23.02.19.
//

#ifndef THECAR_SPEEDTEST_H
#define THECAR_SPEEDTEST_H

#include <ctime>

#define SPEEDTEST_(name, code) \
{clock_t start = clock(); \
code ;\
clock_t stop = clock(); \
LOGD << string(name)+" execution time: "+to_string((stop - start) / (double) CLOCKS_PER_SEC)+" sec."; }

#endif //THECAR_SPEEDTEST_H
