#include "force.h"

#include "engine.h"

Vec2 Gravity::apply(const Rocket& rocket) const
{
    return Vec2{0.0, -rocket.mass() * Atmosphere::GRAVITY};
}

Vec2 Thrust::apply(const Rocket& rocket) const
{
    const double magnitude = rocket.engine().thrust(rocket);
    if (magnitude == 0.0)
    {
        return Vec2{};
    }
    Vec2 direction = unitFromAngle(rocket.launchAngle());
    if (rocket.position().y > Atmosphere::PITCH_OVER_ALTITUDE && length(rocket.velocity()) > 1.0)
    {
        direction = normalised(rocket.velocity());
    }
    return direction * magnitude;
}

Drag::Drag(std::shared_ptr<const Atmosphere> atmosphere) : atmosphere_(std::move(atmosphere)) {}

Vec2 Drag::apply(const Rocket& rocket) const
{
    const Vec2 wind{atmosphere_->windAt(rocket.position().y), 0.0};
    const Vec2 relative = rocket.velocity() - wind;
    const double speed = length(relative);
    if (speed == 0.0)
    {
        return Vec2{};
    }
    const double rho = atmosphere_->density(rocket.position().y);
    const double magnitude = 0.5 * rho * rocket.dragCoeff() * rocket.area() * speed * speed;
    return -normalised(relative) * magnitude;
}
