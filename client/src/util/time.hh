#ifndef _TIME_HH_
#define _TIME_HH_

#include <sys/time.h>
#include <stdint.h>
#include <ratio>
#include <chrono>
#include <stdio.h>

std::time_t getTimeStamp();
void printTime(std::time_t timestamp);

#endif