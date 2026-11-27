#include "atmosphere.h"

#include <cmath>

#include "util.h"

Atmosphere::Atmosphere() : wind_{{0.0, 2.0}, {1000.0, 8.0}, {5000.0, 20.0}, {15000.0, 35.0}} {}

Atmosphere::Atmosphere(WindProfile wind) : wind_(std::move(wind)) {}

double Atmosphere::density(double altitude) const
{
    return SEA_LEVEL_DENSITY * std::exp(-clamp(altitude, 0.0, 1.0e9) / SCALE_HEIGHT);
}

double Atmosphere::windAt(double altitude) const
{
    if (wind_.empty())
    {
        return 0.0;
    }
    auto upper = wind_.lower_bound(altitude);
    if (upper == wind_.begin())
    {
        return upper->second;
    }
    if (upper == wind_.end())
    {
        return std::prev(upper)->second;
    }
    auto lower = std::prev(upper);
    const double t = (altitude - lower->first) / (upper->first - lower->first);
    return lerp(lower->second, upper->second, t);
}
