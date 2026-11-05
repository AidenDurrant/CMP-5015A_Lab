// telemetry.h -- Week 7 (Hard) model solution: a C-style dynamic array of
// samples using new[] / delete[]. Week 8 turns this into a class with the rule
// of three; week 9 replaces it with std::vector<Sample>.

#pragma once

#include <string>

struct Sample
{
    double time;
    double x;
    double y;
    double speed;
    double fuel;
};

struct TelemetryLog
{
    Sample* samples;
    int size;
    int capacity;
};

// Allocate a log able to hold `capacity` samples before it has to grow.
TelemetryLog createLog(int capacity);

// Add a sample, doubling the storage when it is full.
void append(TelemetryLog& log, const Sample& sample);

// Write every sample as one CSV line. Returns false if the file cannot be opened.
bool writeCsv(const TelemetryLog& log, const std::string& path);

// Free the storage. The log must not be used afterwards.
void destroyLog(TelemetryLog& log);
