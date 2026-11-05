// telemetry.cpp -- Week 7 (Hard) model solution.

#include "telemetry.h"

#include <fstream>

TelemetryLog createLog(int capacity)
{
    TelemetryLog log;
    log.samples = new Sample[capacity];
    log.size = 0;
    log.capacity = capacity;
    return log;   // NB: copies the pointer. Fine here, dangerous in general (week 8).
}

void append(TelemetryLog& log, const Sample& sample)
{
    if (log.size == log.capacity)
    {
        const int newCapacity = log.capacity * 2;
        Sample* bigger = new Sample[newCapacity];
        for (int i = 0; i < log.size; ++i)
        {
            bigger[i] = log.samples[i];
        }
        delete[] log.samples;
        log.samples = bigger;
        log.capacity = newCapacity;
    }
    log.samples[log.size] = sample;
    ++log.size;
}

bool writeCsv(const TelemetryLog& log, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
    {
        return false;
    }
    out << "time,x,y,speed,fuel\n";
    for (int i = 0; i < log.size; ++i)
    {
        const Sample& s = log.samples[i];
        out << s.time << ',' << s.x << ',' << s.y << ',' << s.speed << ',' << s.fuel << '\n';
    }
    return true;
}

void destroyLog(TelemetryLog& log)
{
    delete[] log.samples;
    log.samples = nullptr;
    log.size = 0;
    log.capacity = 0;
}
