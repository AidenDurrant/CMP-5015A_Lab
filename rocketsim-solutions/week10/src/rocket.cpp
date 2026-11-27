// rocket.cpp -- Week 10.

#include "rocket.h"

#include <iomanip>
#include <iostream>
#include <numeric>

#include "engine.h"

Rocket::Rocket(const RocketSpec& spec)
    : name_(spec.name),
      payloadMass_(spec.payloadMass),
      stages_(spec.stages),
      dragCoeff_(spec.dragCoeff),
      area_(spec.area),
      launchAngleDeg_(spec.launchAngleDeg),
      engine_(makeEngine(spec.engineType))
{
}

// The destructor and moves are defaulted here rather than in the header: a
// unique_ptr<Engine> can only be destroyed where Engine is a complete type.
Rocket::~Rocket() = default;
Rocket::Rocket(Rocket&&) noexcept = default;
Rocket& Rocket::operator=(Rocket&&) noexcept = default;

double Rocket::mass() const
{
    return std::accumulate(stages_.begin(), stages_.end(), payloadMass_,
                           [](double sum, const Stage& s) { return sum + s.dryMass + s.fuelMass; });
}

double Rocket::fuel() const
{
    return stages_.empty() ? 0.0 : stages_.front().fuelMass;
}

void Rocket::update(const Vec2& totalForce, double dt)
{
    separated_ = false;
    const Vec2 acceleration = totalForce / mass();
    velocity_ += acceleration * dt;
    position_ += velocity_ * dt;

    if (engineRunning())
    {
        Stage& active = stages_.front();
        active.fuelMass -= engine_->burnRate(*this) * dt;
        if (active.fuelMass <= 0.0)
        {
            active.fuelMass = 0.0;
            if (stages_.size() > 1)
            {
                stages_.erase(stages_.begin());   // drop the empty stage
                separated_ = true;
            }
        }
    }
    time_ += dt;
}

void Rocket::jettison(double massKg)
{
    payloadMass_ = payloadMass_ > massKg ? payloadMass_ - massKg : 0.0;
}

std::ostream& operator<<(std::ostream& out, const Rocket& rocket)
{
    return out << std::fixed << std::setprecision(1) << rocket.name_ << "  t=" << std::setw(7)
               << rocket.time_ << " s  alt=" << std::setw(9) << rocket.position_.y
               << " m  range=" << std::setw(9) << rocket.position_.x << " m  speed=" << std::setw(7)
               << length(rocket.velocity_) << " m/s  fuel=" << std::setw(6) << rocket.fuel()
               << " kg  stages=" << rocket.stages_.size();
}
