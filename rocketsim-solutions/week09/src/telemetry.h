// telemetry.h -- Week 9: the log is a std::vector<Sample>. Compare this file
// with the week 8 TelemetryLog class: the destructor, copy constructor, copy
// assignment and capacity doubling have all gone, and nothing was lost.

#pragma once

#include <string>
#include <vector>

struct Sample
{
    double time;
    double x;
    double y;
    double speed;
    double fuel;
};

using TelemetryLog = std::vector<Sample>;

bool writeCsv(const TelemetryLog& log, const std::string& path);

// Week 9 (Easy/Hard): a few <algorithm> summaries of a log.
void printLogSummary(const TelemetryLog& log, double dt);
