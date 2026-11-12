// telemetry.cpp -- Week 8.

#include "telemetry.h"

#include <fstream>

int TelemetryLog::copyCount_ = 0;

TelemetryLog::TelemetryLog(int capacity)
    : samples_(new Sample[capacity]), size_(0), capacity_(capacity)
{
}

TelemetryLog::TelemetryLog(const TelemetryLog& other)
    : samples_(new Sample[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
{
    for (int i = 0; i < size_; ++i)
    {
        samples_[i] = other.samples_[i];
    }
    ++copyCount_;
}

TelemetryLog& TelemetryLog::operator=(const TelemetryLog& other)
{
    if (this != &other)   // self-assignment would delete the array we then copy from
    {
        Sample* fresh = new Sample[other.capacity_];
        for (int i = 0; i < other.size_; ++i)
        {
            fresh[i] = other.samples_[i];
        }
        delete[] samples_;
        samples_ = fresh;
        size_ = other.size_;
        capacity_ = other.capacity_;
        ++copyCount_;
    }
    return *this;
}

TelemetryLog::~TelemetryLog()
{
    delete[] samples_;
}

void TelemetryLog::append(const Sample& sample)
{
    if (size_ == capacity_)
    {
        const int newCapacity = capacity_ * 2;
        Sample* bigger = new Sample[newCapacity];
        for (int i = 0; i < size_; ++i)
        {
            bigger[i] = samples_[i];
        }
        delete[] samples_;
        samples_ = bigger;
        capacity_ = newCapacity;
    }
    samples_[size_++] = sample;
}

int TelemetryLog::copyCount()
{
    return copyCount_;
}

bool writeCsv(const TelemetryLog& log, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
    {
        return false;
    }
    out << "time,x,y,speed,fuel\n";
    for (int i = 0; i < log.size(); ++i)
    {
        const Sample& s = log[i];
        out << s.time << ',' << s.x << ',' << s.y << ',' << s.speed << ',' << s.fuel << '\n';
    }
    return true;
}
