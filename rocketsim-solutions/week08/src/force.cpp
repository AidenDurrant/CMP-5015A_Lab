// force.cpp -- Week 8.

#include "force.h"

#include "engine.h"
#include "physics.h"

Vec2 Gravity::apply(const Rocket& rocket) const
{
    return Vec2{0.0, -rocket.mass() * physics::GRAVITY};
}

Vec2 Thrust::apply(const Rocket& rocket) const
{
    const double magnitude = rocket.engine().thrust(rocket);
    if (magnitude == 0.0)
    {
        return Vec2{};
    }
    Vec2 direction = unitFromAngle(rocket.launchAngle());
    if (rocket.position().y > physics::PITCH_OVER_ALTITUDE && length(rocket.velocity()) > 1.0)
    {
        direction = normalised(rocket.velocity());
    }
    return direction * magnitude;
}

Drag::Drag(const Vec2& wind) : wind_(wind) {}

Vec2 Drag::apply(const Rocket& rocket) const
{
    const Vec2 relative = rocket.velocity() - wind_;
    const double speed = length(relative);
    if (speed == 0.0)
    {
        return Vec2{};
    }
    const double rho = physics::airDensity(rocket.position().y);
    const double magnitude = 0.5 * rho * rocket.dragCoeff() * rocket.area() * speed * speed;
    return -normalised(relative) * magnitude;
}
