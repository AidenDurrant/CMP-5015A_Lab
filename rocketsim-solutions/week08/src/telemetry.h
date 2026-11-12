// telemetry.h -- Week 8 (Medium): the week 7 C-style log is now a class that
// owns its array. Because it owns memory it needs the "rule of three":
// destructor, copy constructor and copy assignment. Try deleting the last two
// and running "TelemetryLog copy = log;" to see the double free they prevent.

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

class TelemetryLog
{
public:
    explicit TelemetryLog(int capacity = 1024);
    TelemetryLog(const TelemetryLog& other);
    TelemetryLog& operator=(const TelemetryLog& other);
    ~TelemetryLog();

    void append(const Sample& sample);
    int size() const { return size_; }
    const Sample& operator[](int i) const { return samples_[i]; }

    static int copyCount();   // how many deep copies have been made (lab demo)

private:
    Sample* samples_;
    int size_;
    int capacity_;
    static int copyCount_;
};

bool writeCsv(const TelemetryLog& log, const std::string& path);
