#include "physics.h"

#include <cmath>

namespace physics
{
    double airDensity(double altitude)
    {
        if (altitude < 0.0)
        {
            altitude = 0.0;
        }
        return SEA_LEVEL_DENSITY * std::exp(-altitude / SCALE_HEIGHT);
    }
}
