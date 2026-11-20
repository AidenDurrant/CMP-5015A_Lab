// wind.h -- Week 9 (Medium): wind speed against altitude in a std::map, with
// linear interpolation between the two nearest entries.

#pragma once

#include <map>

using WindProfile = std::map<double, double>;   // altitude (m) -> wind (m/s, +x)

// operator[] must not be used here: it would insert a zero entry for every
// altitude asked about, silently corrupting the profile.
inline double windAt(const WindProfile& profile, double altitude)
{
    if (profile.empty())
    {
        return 0.0;
    }
    auto upper = profile.lower_bound(altitude);   // first entry >= altitude
    if (upper == profile.begin())
    {
        return upper->second;
    }
    if (upper == profile.end())
    {
        return std::prev(upper)->second;
    }
    auto lower = std::prev(upper);
    const double t = (altitude - lower->first) / (upper->first - lower->first);
    return lower->second + t * (upper->second - lower->second);
}
