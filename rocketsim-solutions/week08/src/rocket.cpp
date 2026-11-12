// rocket.cpp -- Week 8.

#include "rocket.h"

#include <iomanip>
#include <iostream>

#include "engine.h"

int Rocket::liveCount_ = 0;

Rocket::Rocket(const RocketSpec& spec, Engine* engine)
    : name_(spec.name),
      dryMass_(spec.dryMass),
      fuelMass_(spec.fuelMass),
      dragCoeff_(spec.dragCoeff),
      area_(spec.area),
      launchAngleDeg_(spec.launchAngleDeg),
      engine_(engine)
{
    ++liveCount_;
}

Rocket::Rocket(const Rocket& other)
    : name_(other.name_),
      dryMass_(other.dryMass_),
      fuelMass_(other.fuelMass_),
      dragCoeff_(other.dragCoeff_),
      area_(other.area_),
      launchAngleDeg_(other.launchAngleDeg_),
      position_(other.position_),
      velocity_(other.velocity_),
      time_(other.time_),
      engine_(other.engine_)
{
    ++liveCount_;   // the compiler-written copy constructor would not do this
}

Rocket::~Rocket()
{
    --liveCount_;
    std::cout << "Rocket " << name_ << " destroyed\n";   // lab demonstration only
}

void Rocket::update(const Vec2& totalForce, double dt)
{
    const Vec2 acceleration = totalForce / mass();
    velocity_ += acceleration * dt;
    position_ += velocity_ * dt;

    if (engineRunning())
    {
        fuelMass_ -= engine_->burnRate(*this) * dt;
        if (fuelMass_ < 0.0)
        {
            fuelMass_ = 0.0;
        }
    }
    time_ += dt;
}

int Rocket::liveCount()
{
    return liveCount_;
}

std::ostream& operator<<(std::ostream& out, const Rocket& rocket)
{
    return out << std::fixed << std::setprecision(1) << rocket.name_ << "  t=" << std::setw(7)
               << rocket.time_ << " s  alt=" << std::setw(9) << rocket.position_.y
               << " m  range=" << std::setw(9) << rocket.position_.x << " m  speed=" << std::setw(7)
               << length(rocket.velocity_) << " m/s  fuel=" << std::setw(6) << rocket.fuelMass_
               << " kg";
}
