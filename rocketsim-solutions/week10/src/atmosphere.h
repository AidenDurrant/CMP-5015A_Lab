// atmosphere.h -- Week 10 (Medium 2): the air density model and the wind
// profile in one object, shared by every rocket and every Drag force through
// std::shared_ptr. There is one atmosphere; nobody owns it outright.

#pragma once

#include <map>

class Atmosphere
{
public:
    using WindProfile = std::map<double, double>;   // altitude (m) -> wind (m/s, +x)

    Atmosphere();
    explicit Atmosphere(WindProfile wind);

    double density(double altitude) const;
    double windAt(double altitude) const;   // lower_bound + lerp

    static constexpr double GRAVITY = 9.81;
    static constexpr double SEA_LEVEL_DENSITY = 1.225;
    static constexpr double SCALE_HEIGHT = 8500.0;
    static constexpr double PITCH_OVER_ALTITUDE = 300.0;

private:
    WindProfile wind_;
};
